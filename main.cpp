#include <gtk/gtk.h>

typedef enum {
    STOPWATCH_RESET,    // Timer is reset
    STOPWATCH_RUNNING,  // Timer is running
    STOPWATCH_PAUSED    // Timer is paused
} StopwatchState;

static StopwatchState stopwatch_state = STOPWATCH_RESET;  // Initial state is reset
static GTimer *timer = NULL;  // Timer object
static GtkWidget *time_label = NULL;  // Reference to time label to update it
static double paused_time = 0.0;  // Store the paused time to continue from there

// Update the time on the label
static gboolean update_time_label(gpointer user_data) {
    if (stopwatch_state == STOPWATCH_RUNNING && timer) {
        // Get the elapsed time from the timer (in seconds)
        double elapsed = g_timer_elapsed(timer, NULL) + paused_time;
        
        // Convert elapsed time into hours, minutes, and seconds
        int hours = (int)(elapsed / 3600);
        int minutes = (int)((elapsed - (hours * 3600)) / 60);
        int seconds = (int)(elapsed - (hours * 3600) - (minutes * 60));

        // Format time string and update the label
        char time_str[9];
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);
        gtk_label_set_text(GTK_LABEL(time_label), time_str);
    }

    return TRUE; // Continue the timeout
}

// Callback function for Start button
static void on_start_button_clicked(GtkButton *button, gpointer user_data) {
    if (stopwatch_state == STOPWATCH_RESET) {
        // If it's reset, create a new timer and start it
        timer = g_timer_new();
        g_timer_start(timer);
        stopwatch_state = STOPWATCH_RUNNING;  // Change state to running
    } else if (stopwatch_state == STOPWATCH_PAUSED) {
        // If it's paused, resume the existing timer
        g_timer_start(timer);  // Resume the timer
        stopwatch_state = STOPWATCH_RUNNING;  // Change state to running
    }
}

// Callback function for Pause button (Pauses the timer)
static void on_pause_button_clicked(GtkButton *button, gpointer user_data) {
    if (stopwatch_state == STOPWATCH_RUNNING) {
        // Stop the timer and save the paused time
        paused_time += g_timer_elapsed(timer, NULL); // Add elapsed time to paused_time
        g_timer_stop(timer);  // Stop the timer (pause)
        stopwatch_state = STOPWATCH_PAUSED;  // Change state to paused
    }
}

// Callback function for Reset button
static void on_reset_button_clicked(GtkButton *button, gpointer user_data) {
    // If the stopwatch is running or paused, reset it
    stopwatch_state = STOPWATCH_RESET;
    if (timer) {
        g_timer_stop(timer);  // Stop the timer if it's running or paused
        g_timer_reset(timer);  // Reset the timer
    }
    paused_time = 0.0;  // Reset paused time
    gtk_label_set_text(GTK_LABEL(time_label), "00:00:00");  // Reset label text to 00:00:00
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Alex - Stopwatch");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 320);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    GtkWidget *title_label = gtk_label_new("Stopwatch:");
    gtk_widget_add_css_class(title_label, "title-label");
    gtk_box_append(GTK_BOX(vbox), title_label);

    time_label = gtk_label_new("00:00:00");  // Initialize the time label
    gtk_widget_add_css_class(time_label, "time-label");
    gtk_box_append(GTK_BOX(vbox), time_label);

    GtkWidget *start_button = gtk_button_new_with_label("Start");
    gtk_widget_add_css_class(start_button, "button");
    gtk_box_append(GTK_BOX(vbox), start_button);

    GtkWidget *pause_button = gtk_button_new_with_label("Pause");
    gtk_widget_add_css_class(pause_button, "button");
    gtk_box_append(GTK_BOX(vbox), pause_button);

    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    gtk_widget_add_css_class(reset_button, "button");
    gtk_box_append(GTK_BOX(vbox), reset_button);

    // Connect buttons to their callback functions
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), NULL);
    g_signal_connect(pause_button, "clicked", G_CALLBACK(on_pause_button_clicked), NULL);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_button_clicked), NULL);

    // CSS Styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css_provider,
        "window { background-color: #f4f4f4; } "
        ".title-label { font-size: 50px; color:black; font-weight: bold; margin-bottom: 30px; text-decoration: underline; } "
        ".time-label { font-size: 40px; font-weight: bold; } "
        ".button { font-size: 30px; padding: 10px 20px;}"
        ".button:hover {color:blue;}"
       );

    GtkStyleContext *style_context = gtk_widget_get_style_context(GTK_WIDGET(window));
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_window_present(GTK_WINDOW(window));

    // Start a timeout function to update the time every 100 milliseconds
    g_timeout_add(100, update_time_label, NULL);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.stopwatch", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);  // Free the GtkApplication object
    return status;
}
