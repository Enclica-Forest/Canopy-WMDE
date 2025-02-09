#include "widgets/menu_button.h"
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

/* Constants for UI dimensions and styling */
static const int MENU_ICON_SIZE = GTK_ICON_SIZE_MENU;
static const int APP_ICON_SIZE = 48;
static const int POPUP_SPACING = 12;
static const int SEARCH_ENTRY_HEIGHT = 36;
static const int POPOVER_WIDTH = 600;
static const int POPOVER_HEIGHT = 500;
/* Original POPOVER_OFFSET is increased by 20 pixels later to move the popover further below the button */
static const int POPOVER_OFFSET = 10;

typedef struct {
    GtkWidget *list_box;
    GtkWidget *search_entry;
    GList *all_apps;
} AppData;

static gint compare_apps(GAppInfo *a, GAppInfo *b) {
    const gchar *name_a = g_app_info_get_display_name(a);
    const gchar *name_b = g_app_info_get_display_name(b);
    return g_utf8_collate(name_a, name_b);
}

/* Helper function to show error dialog and play a beep sound */
static void show_error_dialog(const gchar *message) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    GdkDisplay *display = gdk_display_get_default();
    if (display) {
        gdk_display_beep(display);
    }
}

/* Callback to launch an application from a list row */
static void on_app_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void) box;
    (void) user_data;
    const gchar *exec_cmd = g_object_get_data(G_OBJECT(row), "exec");
    /* Retrieve the "terminal" flag stored in the row data. */
    gboolean is_terminal = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(row), "terminal"));
    if (exec_cmd) {
        gchar *command;
        if (is_terminal) {
            /* Prepend a terminal emulator command. Adjust the command below if necessary. */
            command = g_strdup_printf("x-terminal-emulator -e %s", exec_cmd);
        } else {
            command = g_strdup(exec_cmd);
        }
        GError *error = NULL;
        if (!g_spawn_command_line_async(command, &error)) {
            show_error_dialog(g_strdup_printf("Error launching application:\n%s", error->message));
            g_error_free(error);
        }
        g_free(command);
    }
}

/* Create a row widget for a given desktop application */
static GtkWidget *create_app_row(GDesktopAppInfo *info) {
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, POPUP_SPACING);
    gtk_widget_set_margin_start(hbox, 12);
    gtk_widget_set_margin_end(hbox, 12);
    gtk_widget_set_margin_top(hbox, 6);
    gtk_widget_set_margin_bottom(hbox, 6);

    GIcon *icon = g_app_info_get_icon(G_APP_INFO(info));
    GtkWidget *image = gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_MENU);
    gtk_widget_set_size_request(image, APP_ICON_SIZE, APP_ICON_SIZE);
    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

    const char *app_name = g_app_info_get_display_name(G_APP_INFO(info));
    GtkWidget *name_label = gtk_label_new(NULL);
    char *markup = g_markup_printf_escaped("<span weight='bold'>%s</span>", app_name);
    gtk_label_set_markup(GTK_LABEL(name_label), markup);
    g_free(markup);
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), name_label, FALSE, FALSE, 0);

    const char *description = g_app_info_get_description(G_APP_INFO(info));
    if (description) {
        GtkWidget *desc_label = gtk_label_new(NULL);
        markup = g_markup_printf_escaped("<span size='small'>%s</span>", description);
        gtk_label_set_markup(GTK_LABEL(desc_label), markup);
        g_free(markup);
        gtk_label_set_line_wrap(GTK_LABEL(desc_label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(desc_label), 50);
        gtk_widget_set_halign(desc_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(vbox), desc_label, FALSE, FALSE, 0);
    }

    gtk_container_add(GTK_CONTAINER(row), hbox);
    
    /* Retrieve the executable command */
    const char *exec_cmd = g_app_info_get_executable(G_APP_INFO(info));
    if (exec_cmd) {
        g_object_set_data_full(G_OBJECT(row), "exec", g_strdup(exec_cmd), g_free);
        /* Check if the app requires a terminal. This reads the "Terminal" key from the .desktop file.
           If Terminal is true, then we'll open a terminal for this application. */
        gboolean is_terminal = g_desktop_app_info_get_boolean(info, "Terminal");
        g_object_set_data(G_OBJECT(row), "terminal", GINT_TO_POINTER(is_terminal));
    }
    
    return row;
}

/* Filter the applications list based on the search entry text */
static void filter_apps(GtkSearchEntry *entry, AppData *data) {
    const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(entry));
    GList *iter;
    
    /* Remove all current rows */
    GList *children = gtk_container_get_children(GTK_CONTAINER(data->list_box));
    while (children) {
        gtk_widget_destroy(GTK_WIDGET(children->data));
        children = g_list_delete_link(children, children);
    }
    
    /* Add filtered rows */
    for (iter = data->all_apps; iter; iter = iter->next) {
        GDesktopAppInfo *info = iter->data;
        const char *app_name = g_app_info_get_display_name(G_APP_INFO(info));
        const char *description = g_app_info_get_description(G_APP_INFO(info));
        
        if (!*search_text || 
            (app_name && strcasestr(app_name, search_text)) ||
            (description && strcasestr(description, search_text))) {
            GtkWidget *row = create_app_row(info);
            gtk_list_box_insert(GTK_LIST_BOX(data->list_box), row, -1);
        }
    }
    
    gtk_widget_show_all(data->list_box);
}

/* Load desktop application files from system directories */
static void load_applications(AppData *data) {
    const gchar * const *data_dirs = g_get_system_data_dirs();
    for (int i = 0; data_dirs[i]; i++) {
        gchar *path = g_build_filename(data_dirs[i], "applications", NULL);
        GDir *dir = g_dir_open(path, 0, NULL);
        if (dir) {
            const char *name;
            while ((name = g_dir_read_name(dir))) {
                if (g_str_has_suffix(name, ".desktop")) {
                    gchar *desktop_file = g_build_filename(path, name, NULL);
                    GDesktopAppInfo *info = g_desktop_app_info_new_from_filename(desktop_file);
                    if (info && g_app_info_should_show(G_APP_INFO(info))) {
                        data->all_apps = g_list_prepend(data->all_apps, info);
                    }
                    g_free(desktop_file);
                }
            }
            g_dir_close(dir);
        }
        g_free(path);
    }
    
    data->all_apps = g_list_sort(data->all_apps, (GCompareFunc)compare_apps);
    filter_apps(GTK_SEARCH_ENTRY(data->search_entry), data);
}

/* System commands with error handling */
static void on_logout_clicked(GtkButton *button, gpointer user_data) {
    (void) button;
    (void) user_data;
    GError *error = NULL;
    if (!g_spawn_command_line_async("gnome-session-quit --logout --no-prompt", &error)) {
        show_error_dialog(g_strdup_printf("Error logging out:\n%s", error->message));
        g_error_free(error);
    }
}

static void on_shutdown_clicked(GtkButton *button, gpointer user_data) {
    (void) button;
    (void) user_data;
    GError *error = NULL;
    if (!g_spawn_command_line_async("systemctl poweroff", &error)) {
        show_error_dialog(g_strdup_printf("Error shutting down:\n%s", error->message));
        g_error_free(error);
    }
}

static void on_restart_clicked(GtkButton *button, gpointer user_data) {
    (void) button;
    (void) user_data;
    GError *error = NULL;
    if (!g_spawn_command_line_async("systemctl reboot", &error)) {
        show_error_dialog(g_strdup_printf("Error restarting:\n%s", error->message));
        g_error_free(error);
    }
}

/* Style a system button with an icon and a label */
static void style_system_button(GtkWidget *button, const char *icon_name, const char *label_text) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *icon = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_DIALOG);
    GtkWidget *label = gtk_label_new(NULL);
    char *markup = g_markup_printf_escaped("<span size='large'>%s</span>", label_text);
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);
    
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    
    gtk_widget_set_margin_start(button, 12);
    gtk_widget_set_margin_end(button, 12);
    gtk_widget_set_margin_top(button, 8);
    gtk_widget_set_margin_bottom(button, 8);
    
    gtk_container_add(GTK_CONTAINER(button), box);
}

/* Create a new menu button with an attached popover */
GtkWidget *menu_button_new(void) {
    GtkWidget *button = gtk_menu_button_new();
    GtkWidget *menu_icon = gtk_image_new_from_icon_name("open-menu-symbolic", MENU_ICON_SIZE);
    gtk_container_add(GTK_CONTAINER(button), menu_icon);

    GtkWidget *popover = gtk_popover_new(button);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(button), popover);
    gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_BOTTOM);
    gtk_popover_set_constrain_to(GTK_POPOVER(popover), GTK_POPOVER_CONSTRAINT_NONE);

    GtkAllocation alloc;
    gtk_widget_get_allocation(button, &alloc);
    GdkRectangle rect = {
        .x = alloc.x + alloc.width / 2 - 1,
        .y = alloc.y + alloc.height + POPOVER_OFFSET + 20,
        .width = 2,
        .height = 2
    };
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_widget_set_margin_top(notebook, 12);
    gtk_widget_set_margin_bottom(notebook, 12);
    gtk_widget_set_margin_start(notebook, 12);
    gtk_widget_set_margin_end(notebook, 12);
    gtk_widget_set_size_request(notebook, POPOVER_WIDTH, POPOVER_HEIGHT);
    gtk_container_add(GTK_CONTAINER(popover), notebook);

    AppData *app_data = g_new0(AppData, 1);
    GtkWidget *apps_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, POPUP_SPACING);
    gtk_widget_set_margin_top(apps_vbox, 6);
    gtk_widget_set_margin_bottom(apps_vbox, 6);
    
    GtkWidget *search = gtk_search_entry_new();
    gtk_widget_set_margin_start(search, 12);
    gtk_widget_set_margin_end(search, 12);
    gtk_widget_set_size_request(search, -1, SEARCH_ENTRY_HEIGHT);
    gtk_box_pack_start(GTK_BOX(apps_vbox), search, FALSE, FALSE, 0);
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_margin_top(scroll, 6);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(apps_vbox), scroll, TRUE, TRUE, 0);
    
    GtkWidget *list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_SINGLE);
    gtk_container_add(GTK_CONTAINER(scroll), list_box);
    
    app_data->list_box = list_box;
    app_data->search_entry = search;
    
    g_signal_connect(search, "search-changed", G_CALLBACK(filter_apps), app_data);
    
    load_applications(app_data);
    
    g_signal_connect(list_box, "row-activated", G_CALLBACK(on_app_row_activated), NULL);

    GtkWidget *apps_label = gtk_label_new("Applications");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), apps_vbox, apps_label);

    GtkWidget *system_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(system_grid), POPUP_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(system_grid), POPUP_SPACING);
    gtk_widget_set_margin_top(system_grid, 24);
    gtk_widget_set_margin_bottom(system_grid, 24);
    gtk_widget_set_margin_start(system_grid, 24);
    gtk_widget_set_margin_end(system_grid, 24);

    GtkWidget *logout_button = gtk_button_new();
    GtkWidget *shutdown_button = gtk_button_new();
    GtkWidget *restart_button = gtk_button_new();

    style_system_button(logout_button, "system-log-out", "Logout");
    style_system_button(shutdown_button, "system-shutdown", "Shutdown");
    style_system_button(restart_button, "system-reboot", "Restart");

    g_signal_connect(logout_button, "clicked", G_CALLBACK(on_logout_clicked), NULL);
    g_signal_connect(shutdown_button, "clicked", G_CALLBACK(on_shutdown_clicked), NULL);
    g_signal_connect(restart_button, "clicked", G_CALLBACK(on_restart_clicked), NULL);

    gtk_grid_attach(GTK_GRID(system_grid), logout_button, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(system_grid), shutdown_button, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(system_grid), restart_button, 0, 1, 1, 1);

    GtkWidget *system_label = gtk_label_new("System");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), system_grid, system_label);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "notebook > header { background: transparent; }"
        "notebook > header > tabs > tab {"
        "    min-height: 30px;"
        "    min-width: 100px;"
        "    margin: 0;"
        "    padding: 4px;"
        "}"
        "list > row:hover { background: alpha(#000, 0.1); }"
        "list > row:selected { background: @theme_selected_bg_color; }"
        "button {"
        "    padding: 12px;"
        "    border-radius: 6px;"
        "}"
        "searchentry {"
        "    border-radius: 6px;"
        "    padding: 6px;"
        "}"
        "scrolledwindow {"
        "    border: none;"
        "}", -1, NULL);
    
    GtkStyleContext *context = gtk_widget_get_style_context(notebook);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    context = gtk_widget_get_style_context(search);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    context = gtk_widget_get_style_context(logout_button);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    context = gtk_widget_get_style_context(shutdown_button);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    context = gtk_widget_get_style_context(restart_button);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);

    g_signal_connect_swapped(popover, "destroy", G_CALLBACK(g_list_free_full), 
                             g_list_copy_deep(app_data->all_apps, (GCopyFunc)g_object_ref, NULL));
    g_signal_connect_swapped(popover, "destroy", G_CALLBACK(g_free), app_data);

    gtk_widget_show_all(notebook);
    return button;
}
