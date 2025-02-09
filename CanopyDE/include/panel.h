#ifndef CANOPY_PANEL_H
#define CANOPY_PANEL_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *menu_button;
    GtkWidget *taskbar;
    GtkWidget *systray;
    GtkWidget *clock;
    GtkWidget *volume;
    GtkWidget *network;
    GtkWidget *battery;
    int height;
    int monitor;
} Panel;

void panel_init(void);
void panel_create(int monitor);
void panel_update(Panel *panel);
void panel_cleanup(void);

#endif