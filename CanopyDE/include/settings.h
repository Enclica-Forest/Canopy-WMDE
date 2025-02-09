#ifndef CANOPY_SETTINGS_H
#define CANOPY_SETTINGS_H

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

typedef struct {
    struct {
        char *theme;
        char *icon_theme;
        char *font;
        int dpi;
    } appearance;
    
    struct {
        char *wallpaper;
        gboolean show_icons;
        gboolean single_click;
    } desktop;
    
    struct {
        int position;
        int size;
        gboolean autohide;
    } panel;
    
    struct {
        char *terminal;
        char *file_manager;
        char *web_browser;
    } default_apps;
} Settings;

void settings_init(void);
void settings_save(void);
void settings_load(void);
Settings *settings_get(void);
void settings_show_dialog(void);

#endif