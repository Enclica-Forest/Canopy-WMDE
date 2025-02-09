#ifndef CANOPY_WIDGET_MENU_BUTTON_H
#define CANOPY_WIDGET_MENU_BUTTON_H

#include <gtk/gtk.h>

/*
 * Creates a new menu button. When clicked, a separate popup window (the menu)
 * appears directly below the button. This menu window contains a search entry and
 * an application list.
 */
GtkWidget *menu_button_new(void);

#endif
