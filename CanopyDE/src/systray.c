#include "systray.h"
#include <X11/Xatom.h>
#include <string.h>
#include <gdk/gdkx.h>  // For gdk_x11_display_get_xdisplay()

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

typedef struct {
    GtkWidget *box;
    GtkWidget *socket_box;
    Window selection_owner;
    Atom selection_atom;
    GHashTable *icons;
} SystemTray;

static SystemTray *tray = NULL;

static GdkFilterReturn systray_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data) {
    XEvent *xev = (XEvent *)xevent;
    
    if (xev->type == ClientMessage) {
        XClientMessageEvent *cev = (XClientMessageEvent *)xev;
        if (cev->message_type == XInternAtom(gdk_x11_display_get_xdisplay(gdk_display_get_default()),
                                              "_NET_SYSTEM_TRAY_OPCODE", False)) {
            switch (cev->data.l[1]) {
                case SYSTEM_TRAY_REQUEST_DOCK: {
                    Window icon_window = cev->data.l[2];
                    GtkWidget *socket = gtk_socket_new();
                    gtk_box_pack_start(GTK_BOX(tray->socket_box), socket, FALSE, FALSE, 0);
                    gtk_widget_show(socket);
                    
                    gtk_socket_add_id(GTK_SOCKET(socket), icon_window);
                    g_hash_table_insert(tray->icons, GUINT_TO_POINTER(icon_window), socket);
                    return GDK_FILTER_REMOVE;
                }
            }
        }
    }
    return GDK_FILTER_CONTINUE;
}

static void acquire_selection(void) {
    /* Use the widget's display to get the X Display.
       This is the recommended method in GTK3.
    */
    Display *display = gdk_x11_display_get_xdisplay(gtk_widget_get_display(tray->box));
    if (!display) {
        g_warning("acquire_selection: Failed to obtain X display.");
        return;
    }
    
    Window root = DefaultRootWindow(display);
    char atom_name[32];
    
    snprintf(atom_name, sizeof(atom_name), "_NET_SYSTEM_TRAY_S%d",
             DefaultScreen(display));
    tray->selection_atom = XInternAtom(display, atom_name, False);
    
    /* Set the selection owner to our tray widget window. */
    XSetSelectionOwner(display, tray->selection_atom,
                      GDK_WINDOW_XID(gtk_widget_get_window(tray->box)),
                      CurrentTime);
                      
    if (XGetSelectionOwner(display, tray->selection_atom) ==
        GDK_WINDOW_XID(gtk_widget_get_window(tray->box))) {
        XClientMessageEvent ev = {0};
        ev.type = ClientMessage;
        ev.window = root;
        ev.message_type = XInternAtom(display, "MANAGER", False);
        ev.format = 32;
        ev.data.l[0] = CurrentTime;
        ev.data.l[1] = tray->selection_atom;
        ev.data.l[2] = GDK_WINDOW_XID(gtk_widget_get_window(tray->box));
        
        XSendEvent(display, root, False, StructureNotifyMask, (XEvent *)&ev);
    }
}

void systray_init(void) {
    if (!tray) {
        tray = g_new0(SystemTray, 1);
        tray->icons = g_hash_table_new_full(g_direct_hash, g_direct_equal,
                                              NULL, (GDestroyNotify)gtk_widget_destroy);
        gdk_window_add_filter(NULL, systray_filter, NULL);
    }
}

GtkWidget *systray_new(void) {
    if (!tray) {
        systray_init();
    }
    
    tray->box = gtk_event_box_new();
    tray->socket_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    
    gtk_container_add(GTK_CONTAINER(tray->box), tray->socket_box);
    gtk_widget_show_all(tray->box);
    
    g_signal_connect(tray->box, "realize", G_CALLBACK(acquire_selection), NULL);
    
    return tray->box;
}

void systray_cleanup(void) {
    if (tray) {
        gdk_window_remove_filter(NULL, systray_filter, NULL);
        if (tray->icons) {
            g_hash_table_destroy(tray->icons);
        }
        g_free(tray);
        tray = NULL;
    }
}
