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
static GtkWidget *start_button = NULL; // Reference to the start/pause button

// Update the time on the label
static gboolean update_time_label(gpointer user_data) {
    if (stopwatch_state == STOPWATCH_RUNNING && timer) {
        double elapsed = g_timer_elapsed(timer, NULL) + paused_time;
        
        int hours = (int)(elapsed / 3600);
        int minutes = (int)((elapsed - (hours * 3600)) / 60);
        int seconds = (int)(elapsed - (hours * 3600) - (minutes * 60));

        char time_str[9];
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);
        gtk_label_set_text(GTK_LABEL(time_label), time_str);
    }

    return TRUE;
}

// Callback for Start/Pause/Continue button
static void on_start_button_clicked(GtkButton *button, gpointer user_data) {
    if (stopwatch_state == STOPWATCH_RESET) {
        timer = g_timer_new();
        g_timer_start(timer);
        stopwatch_state = STOPWATCH_RUNNING;
        gtk_button_set_label(GTK_BUTTON(start_button), "Pause"); //The three stages start/ pause / contine then it goes back to pause
    } else if (stopwatch_state == STOPWATCH_RUNNING) {              //But if you select reset it goes back to start.
        paused_time += g_timer_elapsed(timer, NULL);
        g_timer_stop(timer);
        stopwatch_state = STOPWATCH_PAUSED;
        gtk_button_set_label(GTK_BUTTON(start_button), "Continue");
    } else if (stopwatch_state == STOPWATCH_PAUSED) {
        g_timer_start(timer);
        stopwatch_state = STOPWATCH_RUNNING;
        gtk_button_set_label(GTK_BUTTON(start_button), "Pause");
    }
}

// Callback for Reset button
static void on_reset_button_clicked(GtkButton *button, gpointer user_data) {
    stopwatch_state = STOPWATCH_RESET;
    if (timer) {
        g_timer_stop(timer);
        g_timer_reset(timer);
    }
    paused_time = 0.0;
    gtk_label_set_text(GTK_LABEL(time_label), "00:00:00");
    gtk_button_set_label(GTK_BUTTON(start_button), "Start"); //When the reset button is pressed it will reset the start button back to start to redo the step method.
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Alex 2.0 - Stopwatch Application");
    gtk_window_set_default_size(GTK_WINDOW(window), 430, 320);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    // Set Window Icon
    // gtk_window_set_icon_name(GTK_WINDOW(window),"start-here");


    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    GtkWidget *title_label = gtk_label_new("Stopwatch:");
    gtk_widget_add_css_class(title_label, "title-label");
    gtk_box_append(GTK_BOX(vbox), title_label);

    time_label = gtk_label_new("00:00:00");
    gtk_widget_add_css_class(time_label, "time-label");
    gtk_box_append(GTK_BOX(vbox), time_label);

    start_button = gtk_button_new_with_label("Start");
    gtk_widget_add_css_class(start_button, "button");
    gtk_box_append(GTK_BOX(vbox), start_button);

    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    gtk_widget_add_css_class(reset_button, "button");
    gtk_box_append(GTK_BOX(vbox), reset_button);

    // Connect buttons to callback functions
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), NULL);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_button_clicked), NULL);

    // CSS Styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css_provider,
        "window { background: linear-gradient(to left, #A9D6FF, #69C0FF); }"
        ".title-label { font-size: 60px; color:black; font-weight: bold; margin-bottom: 10px; text-decoration: underline; } "
        ".time-label { font-size: 60px; font-weight: bold; color: #333333;} "
        ".button { font-size: 30px; padding: 10px 20px; border: 4px solid black;background: #69C0FF; color:black}"
        ".button:hover { background: #4da6e0; }"); //background-color: does not work and it needs to be background to assign a background color.

    GtkStyleContext *style_context = gtk_widget_get_style_context(GTK_WIDGET(window));
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_window_present(GTK_WINDOW(window));

    g_timeout_add(100, update_time_label, NULL);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.stopwatch", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
