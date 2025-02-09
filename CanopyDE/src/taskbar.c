#include "taskbar.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>        // For XTextProperty and XGetWMName
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>         // For gdk_x11_display_get_xdisplay()

/* 
 * The structures TaskbarWindow and Taskbar are defined in taskbar.h.
 * We use those definitions here.
 */

static Taskbar *taskbar = NULL;

/* Callback for when a taskbar button is clicked */
static void on_button_clicked(GtkButton *button, TaskbarWindow *win) {
    Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
    if (win->active) {
        // Minimize window if it's already active.
        XIconifyWindow(display, win->window, DefaultScreen(display));
    } else {
        // Activate window.
        XEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.xclient.window = win->window;
        ev.xclient.message_type = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = 2; // Source indication: 2 = pager
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(display, DefaultRootWindow(display), False,
                   SubstructureNotifyMask | SubstructureRedirectMask, &ev);
        XRaiseWindow(display, win->window);
    }
}

/* Find the TaskbarWindow for a given X window */
static TaskbarWindow *find_taskbar_window(Window window) {
    GList *l;
    for (l = taskbar->windows; l; l = l->next) {
        TaskbarWindow *win = l->data;
        if (win->window == window)
            return win;
    }
    return NULL;
}

/* Update the visual state of a taskbar button */
static void update_button_state(TaskbarWindow *win) {
    gtk_widget_set_name(win->button, win->active ? "active-task" : "normal-task");
    GtkStyleContext *context = gtk_widget_get_style_context(win->button);
    if (win->active)
        gtk_style_context_add_class(context, "active");
    else
        gtk_style_context_remove_class(context, "active");
}

/* Create a new taskbar widget */
GtkWidget *taskbar_new(void) {
    taskbar = g_new0(Taskbar, 1);
    taskbar->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    taskbar->windows = NULL;
    taskbar->active_window = None;

    /* Add a CSS style class "taskbar" for custom styling. */
    gtk_style_context_add_class(gtk_widget_get_style_context(taskbar->box), "taskbar");

    /* Allow the taskbar container to expand horizontally */
    gtk_widget_set_hexpand(taskbar->box, TRUE);
    gtk_widget_set_vexpand(taskbar->box, FALSE);

    /* Optionally add a CSS provider for additional styling. */
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        " .taskbar { padding: 2px; } \n"
        " .taskbar button { padding: 2px 6px; } \n"
        " .active-task { background-color: rgba(255,255,255,0.1); } \n",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    return taskbar->box;
}

/* Add a window to the taskbar */
void taskbar_add_window(Window window) {
    if (!taskbar || find_taskbar_window(window))
        return;

    TaskbarWindow *win = g_new0(TaskbarWindow, 1);
    win->window = window;
    win->button = gtk_button_new_with_label("");
    win->active = FALSE;

    g_signal_connect(win->button, "clicked", G_CALLBACK(on_button_clicked), win);

    /* Pack the button into the taskbar container */
    gtk_box_pack_start(GTK_BOX(taskbar->box), win->button, TRUE, TRUE, 0);

    taskbar->windows = g_list_append(taskbar->windows, win);
    taskbar_update_window_title(window);
    gtk_widget_show_all(win->button);
}

/* Remove a window from the taskbar */
void taskbar_remove_window(Window window) {
    TaskbarWindow *win = find_taskbar_window(window);
    if (win) {
        gtk_widget_destroy(win->button);
        g_free(win->title);
        taskbar->windows = g_list_remove(taskbar->windows, win);
        g_free(win);
    }
}

/* Update the title of a window in the taskbar */
void taskbar_update_window_title(Window window) {
    TaskbarWindow *win = find_taskbar_window(window);
    if (!win)
        return;

    Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
    XTextProperty text_prop;
    if (XGetWMName(display, window, &text_prop)) {
        g_free(win->title);
        win->title = g_strdup((char *)text_prop.value);
        gtk_button_set_label(GTK_BUTTON(win->button), win->title);
        XFree(text_prop.value);
    }
}

/* Set the active window in the taskbar */
void taskbar_set_active_window(Window window) {
    if (taskbar->active_window == window)
        return;

    if (taskbar->active_window != None) {
        TaskbarWindow *old_win = find_taskbar_window(taskbar->active_window);
        if (old_win) {
            old_win->active = FALSE;
            update_button_state(old_win);
        }
    }

    taskbar->active_window = window;
    if (window != None) {
        TaskbarWindow *new_win = find_taskbar_window(window);
        if (new_win) {
            new_win->active = TRUE;
            update_button_state(new_win);
        }
    }
}

/* Update the taskbar (e.g., refresh window titles) */
void taskbar_update(GtkWidget *taskbar_widget) {
    if (!taskbar)
        return;

    GList *l;
    for (l = taskbar->windows; l; l = l->next) {
        TaskbarWindow *win = l->data;
        taskbar_update_window_title(win->window);
    }
}

/* Clean up taskbar resources */
void taskbar_cleanup(void) {
    if (taskbar) {
        g_list_free_full(taskbar->windows, (GDestroyNotify)g_free);
        g_free(taskbar);
        taskbar = NULL;
    }
}
