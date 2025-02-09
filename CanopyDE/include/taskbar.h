#ifndef TASKBAR_H
#define TASKBAR_H

#include <gtk/gtk.h>
#include <X11/Xlib.h>

/* Structure representing a window in the taskbar */
typedef struct _TaskbarWindow {
    Window window;      /* The X11 window ID of the client */
    GtkWidget *button;  /* The taskbar button widget */
    gchar *title;       /* The window title */
    gboolean active;    /* TRUE if this window is active */
} TaskbarWindow;

/* Structure representing the taskbar itself */
typedef struct _Taskbar {
    GtkWidget *box;     /* The container widget for the taskbar */
    GList *windows;     /* A list of TaskbarWindow* */
    Window active_window; /* Currently active window */
} Taskbar;

/* Function declarations */
GtkWidget *taskbar_new(void);
void taskbar_add_window(Window window);
void taskbar_remove_window(Window window);
void taskbar_update_window_title(Window window);
void taskbar_set_active_window(Window window);
void taskbar_update(GtkWidget *taskbar_widget);
void taskbar_cleanup(void);

#endif /* TASKBAR_H */
