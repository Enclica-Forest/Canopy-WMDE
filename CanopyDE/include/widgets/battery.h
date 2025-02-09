#ifndef CANOPY_WIDGET_BATTERY_H
#define CANOPY_WIDGET_BATTERY_H

#include <gtk/gtk.h>
#include <libupower-glib/upower.h>

GtkWidget *battery_new(void);
void battery_update(GtkWidget *battery);

#endif