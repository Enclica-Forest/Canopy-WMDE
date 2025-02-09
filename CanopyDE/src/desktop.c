#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include "desktop.h"
#include "settings.h"

typedef struct {
    GtkWidget    *window;
    GtkWidget    *overlay;         /* Contains both the background and icon view */
    GtkWidget    *background_area; /* Custom drawing area for wallpaper */
    GtkWidget    *icon_view;
    GtkListStore *store;
    char         *wallpaper;       /* Path to wallpaper file */
    GdkPixbuf    *wallpaper_pixbuf;/* Loaded wallpaper pixbuf */
} Desktop;

static Desktop desktop;


static gboolean on_draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    Desktop *desk = data;
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width  = allocation.width;
    int height = allocation.height;

    if (desk->wallpaper_pixbuf) {
        double pix_w = gdk_pixbuf_get_width(desk->wallpaper_pixbuf);
        double pix_h = gdk_pixbuf_get_height(desk->wallpaper_pixbuf);
        double scale_x = (double)width / pix_w;
        double scale_y = (double)height / pix_h;
        double scale = (scale_x > scale_y) ? scale_x : scale_y;
        
        cairo_save(cr);
        cairo_scale(cr, scale, scale);
        gdk_cairo_set_source_pixbuf(cr, desk->wallpaper_pixbuf, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    } else {
        /* No wallpaper: fill with a dark grey color */
        cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
        cairo_paint(cr);
    }
    return FALSE;
}

/* Loads desktop icons from the user's desktop directory and adds them to the list store. */
void desktop_load_icons(void)
{
    const char *desktop_dir = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
    if (!desktop_dir) {
        g_warning("No desktop directory found");
        return;
    }
    GDir *dir = g_dir_open(desktop_dir, 0, NULL);
    if (!dir)
        return;
    const char *name;
    while ((name = g_dir_read_name(dir))) {
        char *path = g_build_filename(desktop_dir, name, NULL);
        if (g_str_has_suffix(name, ".desktop")) {
            GDesktopAppInfo *info = g_desktop_app_info_new_from_filename(path);
            if (info) {
                GtkTreeIter iter;
                GIcon *icon = g_app_info_get_icon(G_APP_INFO(info));
                GdkPixbuf *pixbuf = NULL;
                if (icon) {
                    GtkIconTheme *theme = gtk_icon_theme_get_default();
                    GtkIconInfo *icon_info = gtk_icon_theme_lookup_by_gicon(theme, icon, 48, 0);
                    if (icon_info) {
                        pixbuf = gtk_icon_info_load_icon(icon_info, NULL);
                        g_object_unref(icon_info);
                    }
                }
                gtk_list_store_append(desktop.store, &iter);
                gtk_list_store_set(desktop.store, &iter,
                                   0, pixbuf,
                                   1, g_app_info_get_name(G_APP_INFO(info)),
                                   2, g_app_info_get_executable(G_APP_INFO(info)),
                                   -1);
                if (pixbuf)
                    g_object_unref(pixbuf);
                g_object_unref(info);
            }
        }
        g_free(path);
    }
    g_dir_close(dir);
}

/* Sets the wallpaper by loading the image file and updating the background area. */
void desktop_set_wallpaper(const char *path)
{
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, &error);
    if (!pixbuf) {
        g_warning("Failed to load wallpaper '%s': %s", path, error->message);
        g_error_free(error);
        return;
    }
    
    if (desktop.wallpaper_pixbuf)
        g_object_unref(desktop.wallpaper_pixbuf);
    desktop.wallpaper_pixbuf = pixbuf;
    
    g_free(desktop.wallpaper);
    desktop.wallpaper = g_strdup(path);
    
    gtk_widget_queue_draw(desktop.background_area);
}

void desktop_init(void)
{
    desktop.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(desktop.window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(desktop.window), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(desktop.window), GDK_WINDOW_TYPE_HINT_DESKTOP);
    gtk_window_fullscreen(GTK_WINDOW(desktop.window));

    desktop.overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(desktop.window), desktop.overlay);
    
    /* Create and add the background drawing area */
    desktop.background_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(desktop.background_area, TRUE);
    gtk_widget_set_vexpand(desktop.background_area, TRUE);
    g_signal_connect(desktop.background_area, "draw", G_CALLBACK(on_draw_background), &desktop);
    gtk_container_add(GTK_CONTAINER(desktop.overlay), desktop.background_area);
    
    /* Create the icon view (desktop icons) */
    desktop.store = gtk_list_store_new(3, 
        GDK_TYPE_PIXBUF,  /* Column 0: icon */
        G_TYPE_STRING,    /* Column 1: name */
        G_TYPE_STRING     /* Column 2: executable command */
    );
    desktop.icon_view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(desktop.store));
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(desktop.icon_view), 0);
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(desktop.icon_view), 1);
    gtk_icon_view_set_item_width(GTK_ICON_VIEW(desktop.icon_view), 100);
    
    gtk_overlay_add_overlay(GTK_OVERLAY(desktop.overlay), desktop.icon_view);
    
    /* Load the desktop icons */
    desktop_load_icons();
    
    gtk_widget_show_all(desktop.window);
}
