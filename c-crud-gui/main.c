#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

sqlite3 *db;
GtkWidget *entry_id, *entry_name, *entry_age, *entry_search, *textview_output;

// Function to execute SQL commands
void execute_sql(const char *sql) {
    char *errmsg;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("Error executing SQL: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
}

// Initialize database and create table if not exists
void init_db() {
    const char *sql_create_table = "CREATE TABLE IF NOT EXISTS people (id INTEGER PRIMARY KEY, name TEXT, age INTEGER);";
    execute_sql(sql_create_table);
}

// Display message in TextView
void display_message(const char *message) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_output));
    gtk_text_buffer_set_text(buffer, message, -1);
}

// Create new record
void on_create_clicked(GtkWidget *widget, gpointer data) {
    const char *name = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const char *age = gtk_entry_get_text(GTK_ENTRY(entry_age));

    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO people (name, age) VALUES ('%s', %s);", name, age);
    execute_sql(sql);

    display_message("Record created successfully.");
}

// Read all records and display in the TextView
void on_read_clicked(GtkWidget *widget, gpointer data) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM people;";
    char output[1024] = "";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char *name = (const char *)sqlite3_column_text(stmt, 1);
            int age = sqlite3_column_int(stmt, 2);
            char row[256];
            snprintf(row, sizeof(row), "ID: %d | Name: %s | Age: %d\n", id, name, age);
            strcat(output, row);
        }
        sqlite3_finalize(stmt);
    }

    display_message(output);
}

// Update a record
void on_update_clicked(GtkWidget *widget, gpointer data) {
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry_id));
    const char *name = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const char *age = gtk_entry_get_text(GTK_ENTRY(entry_age));

    char sql[256];
    snprintf(sql, sizeof(sql), "UPDATE people SET name = '%s', age = %s WHERE id = %s;", name, age, id);
    execute_sql(sql);

    display_message("Record updated successfully.");
}

// Delete a record
void on_delete_clicked(GtkWidget *widget, gpointer data) {
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry_id));

    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM people WHERE id = %s;", id);
    execute_sql(sql);

    display_message("Record deleted successfully.");
}

// Search for a record by name
void on_search_clicked(GtkWidget *widget, gpointer data) {
    const char *name = gtk_entry_get_text(GTK_ENTRY(entry_search));
    sqlite3_stmt *stmt;
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM people WHERE name LIKE '%%%s%%';", name);
    char output[1024] = "";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char *name = (const char *)sqlite3_column_text(stmt, 1);
            int age = sqlite3_column_int(stmt, 2);
            char row[256];
            snprintf(row, sizeof(row), "ID: %d | Name: %s | Age: %d\n", id, name, age);
            strcat(output, row);
        }
        sqlite3_finalize(stmt);
    }

    display_message(output);
}

// Main function for creating the GUI and setting up callbacks
int main(int argc, char *argv[]) {
    GtkWidget *window, *grid, *button_create, *button_read, *button_update, *button_delete, *button_search;

    // Initialize GTK
    gtk_init(&argc, &argv);

    // Open SQLite database
    if (sqlite3_open("database.db", &db) != SQLITE_OK) {
        printf("Failed to open database\n");
        return 1;
    }
    init_db();

    // Create a window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CRUD with Search");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a grid layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Entry fields
    entry_id = gtk_entry_new();
    entry_name = gtk_entry_new();
    entry_age = gtk_entry_new();
    entry_search = gtk_entry_new();
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_id, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Name:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_name, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Age:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_age, 1, 2, 1, 1);

    // Buttons for CRUD operations
    button_create = gtk_button_new_with_label("Create");
    button_read = gtk_button_new_with_label("Read");
    button_update = gtk_button_new_with_label("Update");
    button_delete = gtk_button_new_with_label("Delete");
    button_search = gtk_button_new_with_label("Search");

    gtk_grid_attach(GTK_GRID(grid), button_create, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_read, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_update, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_delete, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Search by Name:"), 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_search, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_search, 0, 6, 1, 1);

    // Output text view
    textview_output = gtk_text_view_new();
    gtk_grid_attach(GTK_GRID(grid), textview_output, 0, 7, 2, 2);

    // Connect button signals to functions
    g_signal_connect(button_create, "clicked", G_CALLBACK(on_create_clicked), NULL);
    g_signal_connect(button_read, "clicked", G_CALLBACK(on_read_clicked), NULL);
    g_signal_connect(button_update, "clicked", G_CALLBACK(on_update_clicked), NULL);
    g_signal_connect(button_delete, "clicked", G_CALLBACK(on_delete_clicked), NULL);
    g_signal_connect(button_search, "clicked", G_CALLBACK(on_search_clicked), NULL);

    // Show the window and its contents
    gtk_widget_show_all(window);

    // Main GTK loop
    gtk_main();

    // Close database before exit
    sqlite3_close(db);

    return 0;
}