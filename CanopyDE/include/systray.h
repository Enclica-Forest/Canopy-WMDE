#ifndef CANOPY_SYSTRAY_H
#define CANOPY_SYSTRAY_H

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gtk/gtkx.h>  // Required for GtkSocket

void systray_init(void);
GtkWidget *systray_new(void);
void systray_cleanup(void);

#endif