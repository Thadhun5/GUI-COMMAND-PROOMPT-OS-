#include <gtk/gtk.h>

GtkWidget *text_view;
GtkTextBuffer *text_buffer;
GtkWidget *entry;
GtkWidget *status_label;
GtkEntryCompletion *completion;  // Command autocompletion
GtkWidget *window;  // Declare the main application window

const char *command_suggestions[] = {
    "ls", "cat", "cd", "gcc", "make", "chmod", "chown", "ps", "kill", "mkdir",
    "rm", "touch", "echo", "mv", "cp", "pwd", "find", "grep", "sed", "awk"
};

// Function to set autocompletion model
void setupAutocompletion(GtkEntry *entry, GtkListStore *suggestions) {
    GtkEntryCompletion *completion = gtk_entry_completion_new();
    gtk_entry_completion_set_text_column(completion, 0);
    gtk_entry_set_completion(entry, completion);
    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(suggestions));
    g_object_unref(suggestions);
}

void on_button_clicked(GtkWidget *widget, gpointer data) {
    const gchar *command = gtk_entry_get_text(GTK_ENTRY(entry));
    gchar result[2048] = "";

    FILE *terminal = popen(command, "r");
    char buffer[1024];

    if (terminal) {
        while (fgets(buffer, sizeof(buffer), terminal) != NULL) {
            g_strlcat(result, buffer, sizeof(result));
        }
        pclose(terminal);
        gtk_label_set_text(GTK_LABEL(status_label), "Command executed successfully.");
    } else {
        g_strlcpy(result, "Failed to execute command.", sizeof(result));
        gtk_label_set_text(GTK_LABEL(status_label), "Error: Failed to execute command");
        
        // Add error message dialog
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Error: Failed to execute command");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
    }

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(text_buffer, &end);
    gtk_text_buffer_insert(text_buffer, &end, command, -1);
    gtk_text_buffer_insert(text_buffer, &end, "\n", -1);
    gtk_text_buffer_insert(text_buffer, &end, result, -1);
    gtk_text_buffer_insert(text_buffer, &end, "\n", -1);

    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

void clear_output(GtkWidget *widget, gpointer data) {
    gtk_text_buffer_set_text(text_buffer, "", -1);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  // Create the main application window
    gtk_window_set_title(GTK_WINDOW(window), "Command Prompt GUI");
    gtk_container_set_border_width(GTK_CONTAINER(window), 20); // Increased border width
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 700); // Increased the window size

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10); // Increased row spacing
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10); // Increased column spacing

    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "Enter command here");

    // Initialize autocompletion
    GtkListStore *suggestions = gtk_list_store_new(1, G_TYPE_STRING);
    for (int i = 0; i < sizeof(command_suggestions) / sizeof(command_suggestions[0]); i++) {
        GtkTreeIter iter;
        gtk_list_store_append(suggestions, &iter);
        gtk_list_store_set(suggestions, &iter, 0, command_suggestions[i], -1);
    }

    setupAutocompletion(GTK_ENTRY(entry), suggestions);

    GtkWidget *button = gtk_button_new_with_label("Execute Command");
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_button_clicked), NULL);

    text_view = gtk_text_view_new();
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_widget_set_size_request(scrolled_window, 400, 400); // Increased the size of the output area

    status_label = gtk_label_new("Status: Ready");

    // Arrange widgets in a symmetric layout
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), status_label, 0, 2, 2, 1);

    // Add a "Clear Output" button
    GtkWidget *clear_button = gtk_button_new_with_label("Clear Output");
    g_signal_connect(G_OBJECT(clear_button), "clicked", G_CALLBACK(clear_output), NULL);
    gtk_grid_attach(GTK_GRID(grid), clear_button, 2, 0, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();

    return 0;
}
