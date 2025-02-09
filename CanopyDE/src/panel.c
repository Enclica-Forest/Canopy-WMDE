    #include "panel.h"
    #include "widgets/clock.h"
    #include "widgets/volume.h"
    #include "widgets/network.h"
    #include "widgets/battery.h"
    #include "widgets/menu_button.h"
    #include "taskbar.h"
    #include "systray.h"
    #include <gtk/gtk.h>

    #define PANEL_HEIGHT 30
    static GList *panels = NULL;

    void panel_init(void) {
        GdkDisplay *display = gdk_display_get_default();
        int n_monitors = gdk_display_get_n_monitors(display);
        
        for (int i = 0; i < n_monitors; i++) {
            panel_create(i);
        }
    }

    void panel_create(int monitor) {
        Panel *panel = g_new0(Panel, 1);
        panel->height = PANEL_HEIGHT;
        panel->monitor = monitor;
        
        /* Create panel window */
        panel->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_decorated(GTK_WINDOW(panel->window), FALSE);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(panel->window), TRUE);

        /* Enable transparency for the window */
        gtk_widget_set_app_paintable(panel->window, TRUE);
        GdkScreen *screen = gtk_widget_get_screen(panel->window);
        GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
        if (visual)
            gtk_widget_set_visual(panel->window, visual);
        /* Set window opacity (0.0 fully transparent, 1.0 opaque) */
        gtk_window_set_opacity(GTK_WINDOW(panel->window), 0.8);

        /* Get monitor geometry */
        GdkDisplay *display2 = gdk_display_get_default();
        GdkMonitor *mon = gdk_display_get_monitor(display2, monitor);
        GdkRectangle geometry;
        gdk_monitor_get_geometry(mon, &geometry);
        
        /* Make the panel window fill the entire monitor */
        gtk_window_move(GTK_WINDOW(panel->window), geometry.x, geometry.y);
        gtk_widget_set_size_request(panel->window, geometry.width, geometry.height);

        /* Create a horizontal box with padding */
        panel->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_set_border_width(GTK_CONTAINER(panel->box), 4);
        /* Align the box to the top of the window */
        gtk_widget_set_halign(panel->box, GTK_ALIGN_FILL);
        gtk_widget_set_valign(panel->box, GTK_ALIGN_START);
        gtk_container_add(GTK_CONTAINER(panel->window), panel->box);
        
        /* Initialize widgets */
        panel->menu_button = menu_button_new();
        panel->taskbar = taskbar_new();
        panel->systray = systray_new();
        panel->clock = clock_new();
        panel->volume = volume_new();
        panel->network = network_new();
        panel->battery = battery_new();
        
        /* Set minimum sizes */
        gtk_widget_set_size_request(panel->menu_button, 32, -1);
        gtk_widget_set_size_request(panel->clock, 80, -1);
        gtk_widget_set_size_request(panel->volume, 32, -1);
        gtk_widget_set_size_request(panel->network, 32, -1);
        gtk_widget_set_size_request(panel->battery, 32, -1);
        gtk_widget_set_size_request(panel->systray, 100, -1);
        
        /* Pack widgets: menu button on left, taskbar in the middle */
        gtk_box_pack_start(GTK_BOX(panel->box), panel->menu_button, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(panel->box), panel->taskbar, TRUE, TRUE, 2);
        
        /* Create status box for additional widgets */
        GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_box_pack_start(GTK_BOX(status_box), panel->systray, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(status_box), panel->clock, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(status_box), panel->volume, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(status_box), panel->network, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(status_box), panel->battery, FALSE, FALSE, 2);
        
        gtk_box_pack_end(GTK_BOX(panel->box), status_box, FALSE, FALSE, 2);
        
        gtk_widget_show_all(panel->window);
        panels = g_list_append(panels, panel);
    }

    void panel_update(Panel *panel) {
        clock_update(panel->clock);
        volume_update(panel->volume);
        network_update(panel->network);
        battery_update(panel->battery);
    }

    void panel_cleanup(void) {
        g_list_free_full(panels, g_free);
        panels = NULL;
    }
