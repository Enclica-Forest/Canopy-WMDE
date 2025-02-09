    // src/settings.c
    #include "settings.h"

    static Settings settings;
void settings_load(void) {
    gchar *config_file = g_build_filename(g_get_user_config_dir(), "canopy", "settings.json", NULL);
    
    if (!g_file_test(config_file, G_FILE_TEST_EXISTS)) {
        g_free(config_file);
        return;
    }

    GError *error = NULL;
    JsonParser *parser = json_parser_new();
    json_parser_load_from_file(parser, config_file, &error);
    
    if (error) {
        g_warning("Failed to load settings: %s", error->message);
        g_error_free(error);
        g_object_unref(parser);
        g_free(config_file);
        return;
    }

    JsonNode *root = json_parser_get_root(parser);
    JsonObject *root_obj = json_node_get_object(root);

    // Parse appearance settings
    JsonObject *appearance = json_object_get_object_member(root_obj, "appearance");
    if (appearance) {
        g_free(settings.appearance.theme);
        settings.appearance.theme = g_strdup(json_object_get_string_member(appearance, "theme"));
        settings.appearance.icon_theme = g_strdup(json_object_get_string_member(appearance, "icon_theme"));
        settings.appearance.font = g_strdup(json_object_get_string_member(appearance, "font"));
        settings.appearance.dpi = json_object_get_int_member(appearance, "dpi");
    }

    // Parse desktop settings
    JsonObject *desktop = json_object_get_object_member(root_obj, "desktop");
    if (desktop) {
        settings.desktop.wallpaper = g_strdup(json_object_get_string_member(desktop, "wallpaper"));
        settings.desktop.show_icons = json_object_get_boolean_member(desktop, "show_icons");
        settings.desktop.single_click = json_object_get_boolean_member(desktop, "single_click");
    }

    g_object_unref(parser);
    g_free(config_file);
}
    static void create_settings_window(void) {
        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Canopy Settings");
        gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
        
        GtkWidget *notebook = gtk_notebook_new();
        gtk_container_add(GTK_CONTAINER(window), notebook);
        
        // Appearance page
        GtkWidget *appearance_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_container_set_border_width(GTK_CONTAINER(appearance_page), 12);
        
        // Theme selector
        GtkWidget *theme_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        GtkWidget *theme_label = gtk_label_new("Theme:");
        GtkWidget *theme_combo = gtk_combo_box_text_new();
        
        // Get available themes
        const char *theme_dir = "/usr/share/themes";
        GDir *dir = g_dir_open(theme_dir, 0, NULL);
        if (dir) {
            const char *name;
            while ((name = g_dir_read_name(dir))) {
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(theme_combo), name);
            }
            g_dir_close(dir);
        }
        
        gtk_box_pack_start(GTK_BOX(theme_box), theme_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(theme_box), theme_combo, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(appearance_page), theme_box, FALSE, FALSE, 0);
        
        // Font selector
        GtkWidget *font_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        GtkWidget *font_label = gtk_label_new("Font:");
        GtkWidget *font_button = gtk_font_button_new_with_font(settings.appearance.font);
        
        gtk_box_pack_start(GTK_BOX(font_box), font_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(font_box), font_button, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(appearance_page), font_box, FALSE, FALSE, 0);
        
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), appearance_page,
                            gtk_label_new("Appearance"));
        
        // Desktop page
        GtkWidget *desktop_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_container_set_border_width(GTK_CONTAINER(desktop_page), 12);
        
        // Wallpaper selector
        GtkWidget *wallpaper_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        GtkWidget *wallpaper_label = gtk_label_new("Wallpaper:");
        GtkWidget *wallpaper_chooser = gtk_file_chooser_button_new("Select Wallpaper",
                                                                GTK_FILE_CHOOSER_ACTION_OPEN);
        
        gtk_box_pack_start(GTK_BOX(wallpaper_box), wallpaper_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(wallpaper_box), wallpaper_chooser, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(desktop_page), wallpaper_box, FALSE, FALSE, 0);
        
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), desktop_page,
                            gtk_label_new("Desktop"));
        
        gtk_widget_show_all(window);
    }

    void settings_init(void) {
        // Set defaults
        settings.appearance.theme = g_strdup("Adwaita");
        settings.appearance.icon_theme = g_strdup("hicolor");
        settings.appearance.font = g_strdup("Sans 10");
        settings.appearance.dpi = 96;
        
        settings.desktop.wallpaper = NULL;
        settings.desktop.show_icons = TRUE;
        settings.desktop.single_click = FALSE;
        
        settings.panel.position = 0; // Top
        settings.panel.size = 28;
        settings.panel.autohide = FALSE;
        
        settings.default_apps.terminal = g_strdup("xterm");
        settings.default_apps.file_manager = g_strdup("nautilus");
        settings.default_apps.web_browser = g_strdup("firefox");
        
        settings_load();
    }

    void settings_save(void) {
        JsonBuilder *builder = json_builder_new();
        
        json_builder_begin_object(builder);
        
        // Appearance
        json_builder_set_member_name(builder, "appearance");
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "theme");
        json_builder_add_string_value(builder, settings.appearance.theme);
        json_builder_set_member_name(builder, "icon_theme");
        json_builder_add_string_value(builder, settings.appearance.icon_theme);
        json_builder_set_member_name(builder, "font");
        json_builder_add_string_value(builder, settings.appearance.font);
        json_builder_set_member_name(builder, "dpi");
        json_builder_add_int_value(builder, settings.appearance.dpi);
        json_builder_end_object(builder);
        
        // Desktop
        json_builder_set_member_name(builder, "desktop");
        json_builder_begin_object(builder);
        if (settings.desktop.wallpaper) {
            json_builder_set_member_name(builder, "wallpaper");
            json_builder_add_string_value(builder, settings.desktop.wallpaper);
        }
        json_builder_set_member_name(builder, "show_icons");
        json_builder_add_boolean_value(builder, settings.desktop.show_icons);
        json_builder_set_member_name(builder, "single_click");
        json_builder_add_boolean_value(builder, settings.desktop.single_click);
        json_builder_end_object(builder);
        
        json_builder_end_object(builder);
        
        JsonGenerator *gen = json_generator_new();
        JsonNode *root = json_builder_get_root(builder);
        json_generator_set_root(gen, root);
        
        char *config_dir = g_build_filename(g_get_user_config_dir(), "canopy", NULL);
        g_mkdir_with_parents(config_dir, 0755);
        
        char *config_file = g_build_filename(config_dir, "settings.json", NULL);
        json_generator_to_file(gen, config_file, NULL);
        
        g_free(config_file);
        g_free(config_dir);
        json_node_free(root);
        g_object_unref(gen);
        g_object_unref(builder);
    }

    Settings *settings_get(void) {
        return &settings;
    }

    void settings_show_dialog(void) {
        create_settings_window();
    }