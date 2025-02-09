#include "wm.h"
#include "input.h"
#include "client.h"
#include <X11/Xcursor/Xcursor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <systemd/sd-bus.h>

/* Define UI buffer margins */
#define UI_BUFFER_TOP    32  /* reserved for panel (panel height is 32) */
#define UI_BUFFER_BOTTOM 0
#define UI_BUFFER_LEFT   0
#define UI_BUFFER_RIGHT  0

/* Global WM instance */
WM wm;

/* Initialize common atoms */
static void wm_init_atoms(void) {
    const char *atom_names[] = {
        "WM_PROTOCOLS",
        "WM_DELETE_WINDOW",
        "WM_STATE",
        "WM_TAKE_FOCUS",
        "_NET_WM_NAME",
        "_NET_WM_STATE",
        "_NET_WM_STATE_FULLSCREEN",
        "_NET_ACTIVE_WINDOW",
        "_NET_WM_WINDOW_TYPE",
        "_NET_WM_WINDOW_TYPE_DOCK",
        "_NET_WM_WINDOW_TYPE_TOOLBAR",
        "_NET_WM_WINDOW_TYPE_MENU",
        "_NET_WM_WINDOW_TYPE_UTILITY",
        "_NET_WM_WINDOW_TYPE_SPLASH",
        "_NET_WM_WINDOW_TYPE_DIALOG",
        "_NET_WM_WINDOW_TYPE_NORMAL"
    };

    XInternAtoms(wm.display, (char **)atom_names, ATOM_COUNT, False, wm.atoms);
}

/* Initialize modifier masks */
static void wm_init_masks(void) {
    XModifierKeymap *modmap = XGetModifierMapping(wm.display);
    if (modmap && modmap->max_keypermod > 0) {
        const KeyCode num_lock = XKeysymToKeycode(wm.display, XK_Num_Lock);
        const KeyCode scroll_lock = XKeysymToKeycode(wm.display, XK_Scroll_Lock);
        const KeyCode caps_lock = XKeysymToKeycode(wm.display, XK_Caps_Lock);
        int i;
        for (i = 0; i < 8 * modmap->max_keypermod; i++) {
            if (modmap->modifiermap[i] == num_lock)
                wm.num_lock_mask = 1 << (i / modmap->max_keypermod);
            else if (modmap->modifiermap[i] == scroll_lock)
                wm.scroll_lock_mask = 1 << (i / modmap->max_keypermod);
            else if (modmap->modifiermap[i] == caps_lock)
                wm.caps_lock_mask = 1 << (i / modmap->max_keypermod);
        }
    }
    if (modmap)
        XFreeModifiermap(modmap);
}

/* Initialize the window manager, including the desktop background window */
void wm_init(void) {
    /* Initialize X connection. */
    wm.display = XOpenDisplay(NULL);
    if (!wm.display) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }
    wm.screen = DefaultScreen(wm.display);
    wm.root = RootWindow(wm.display, wm.screen);
    wm.gc = XCreateGC(wm.display, wm.root, 0, NULL);

    /* Create a Cairo surface/context for general WM drawing. */
    int width = DisplayWidth(wm.display, wm.screen);
    int height = DisplayHeight(wm.display, wm.screen);
    wm.surface = cairo_xlib_surface_create(wm.display, wm.root,
                                           DefaultVisual(wm.display, wm.screen),
                                           width, height);
    wm.cr = cairo_create(wm.surface);

    /* Create the desktop background window. */
    wm.desktop_width = width;
    wm.desktop_height = height;
    {
        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        attrs.background_pixel = BlackPixel(wm.display, wm.screen);
        attrs.event_mask = ExposureMask;
        wm.desktop_window = XCreateWindow(wm.display, wm.root,
                                          0, 0, wm.desktop_width, wm.desktop_height,
                                          0, CopyFromParent, InputOutput,
                                          CopyFromParent,
                                          CWOverrideRedirect | CWBackPixel | CWEventMask,
                                          &attrs);
        if (!wm.desktop_window) {
            fprintf(stderr, "Failed to create desktop window\n");
            exit(1);
        }
        Atom net_wm_window_type = XInternAtom(wm.display, "_NET_WM_WINDOW_TYPE", False);
        Atom desktop_type = XInternAtom(wm.display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
        XChangeProperty(wm.display, wm.desktop_window, net_wm_window_type, XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)&desktop_type, 1);
        XMapWindow(wm.display, wm.desktop_window);
        XLowerWindow(wm.display, wm.desktop_window);
        wm.desktop_surface = cairo_xlib_surface_create(wm.display, wm.desktop_window,
                                                       DefaultVisual(wm.display, wm.screen),
                                                       wm.desktop_width, wm.desktop_height);
        wm.desktop_cr = cairo_create(wm.desktop_surface);
    }

    wm_init_atoms();
    wm_init_masks();

    /* Initialize RandR extension. */
    if (!XRRQueryExtension(wm.display, &wm.randr_event_base, &wm.randr_error_base)) {
        fprintf(stderr, "RandR extension not available\n");
        wm.randr_event_base = -1;
        wm.randr_error_base = -1;
    }

    /* Initialize systemd bus connection. */
    int ret = sd_bus_default_system(&wm.bus);
    if (ret < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-ret));
        wm.bus = NULL;
    }

    /* Set input events on the root window. */
    XSelectInput(wm.display, wm.root,
                 SubstructureRedirectMask | SubstructureNotifyMask |
                 ButtonPressMask | KeyPressMask | PropertyChangeMask |
                 EnterWindowMask | LeaveWindowMask | FocusChangeMask);

    wm_grab_keys();
    wm_grab_buttons();

    wm.running = true;
    wm.focused_window = None;
}

/* Cleanup WM resources, including desktop window and Cairo contexts */
void wm_cleanup(void) {
    if (wm.cr) {
        cairo_destroy(wm.cr);
        wm.cr = NULL;
    }
    if (wm.surface) {
        cairo_surface_destroy(wm.surface);
        wm.surface = NULL;
    }
    if (wm.desktop_cr) {
        cairo_destroy(wm.desktop_cr);
        wm.desktop_cr = NULL;
    }
    if (wm.desktop_surface) {
        cairo_surface_destroy(wm.desktop_surface);
        wm.desktop_surface = NULL;
    }
    if (wm.gc) {
        XFreeGC(wm.display, wm.gc);
    }
    if (wm.bus) {
        sd_bus_unref(wm.bus);
    }
    if (wm.display) {
        XCloseDisplay(wm.display);
    }
}

/* 
 * wm_handle_event: dispatches the X event to the appropriate handler.
 */
void wm_handle_event(XEvent *ev) {
    switch (ev->type) {
        case MapRequest:
            wm_handle_map_request(&ev->xmaprequest);
            break;
        case ConfigureRequest:
            wm_handle_configure_request(&ev->xconfigurerequest);
            break;
        case PropertyNotify:
            wm_handle_property_notify(&ev->xproperty);
            break;
        case ClientMessage:
            wm_handle_client_message(&ev->xclient);
            break;
        case DestroyNotify:
            wm_handle_destroy_notify(&ev->xdestroywindow);
            break;
        case EnterNotify:
            wm_handle_enter_notify(&ev->xcrossing);
            break;
        case ButtonPress:
            wm_handle_button_press(&ev->xbutton);
            break;
        case KeyPress:
            wm_handle_key_press(&ev->xkey);
            break;
        default:
            /* Handle additional event types if needed */
            break;
    }
}

/* Main WM event loop */
void wm_run(void) {
    XEvent ev;
    while (wm.running) {
        XNextEvent(wm.display, &ev);
        wm_handle_event(&ev);
        XFlush(wm.display);
    }
}

/* -- Event Handlers -- */

/* When a window is mapped, add it as a client if it is not override-redirect */
void wm_handle_map_request(XMapRequestEvent *ev) {
    XWindowAttributes attr;
    XGetWindowAttributes(wm.display, ev->window, &attr);
    if (!attr.override_redirect) {
        /* Optionally, you could adjust the initial position here too */
        XMapWindow(wm.display, ev->window);
        client_add(ev->window);
    }
}

/* Ensure windows stay within the allowed (non-UI) area before configuring them */
void wm_handle_configure_request(XConfigureRequestEvent *ev) {
    XWindowChanges changes;
    int new_x = ev->x;
    int new_y = ev->y;
    int new_width = ev->width;
    int new_height = ev->height;

    /* Get screen dimensions from the desktop size */
    int screen_width = wm.desktop_width;
    int screen_height = wm.desktop_height;

    /* Adjust horizontal position and width */
    if (new_x < UI_BUFFER_LEFT)
        new_x = UI_BUFFER_LEFT;
    if (new_x + new_width > screen_width - UI_BUFFER_RIGHT)
        new_x = screen_width - UI_BUFFER_RIGHT - new_width;

    /* Adjust vertical position and height */
    if (new_y < UI_BUFFER_TOP)
        new_y = UI_BUFFER_TOP;
    if (new_y + new_height > screen_height - UI_BUFFER_BOTTOM)
        new_y = screen_height - UI_BUFFER_BOTTOM - new_height;

    changes.x = new_x;
    changes.y = new_y;
    changes.width = new_width;
    changes.height = new_height;
    changes.border_width = ev->border_width;
    changes.sibling = ev->above;
    changes.stack_mode = ev->detail;
    XConfigureWindow(wm.display, ev->window, ev->value_mask, &changes);
}

/* Update client title when the window property changes */
void wm_handle_property_notify(XPropertyEvent *ev) {
    if (ev->atom == XA_WM_NAME) {
        Client *c = client_find_by_window(ev->window);
        if (c) {
            client_update_title(c);
        }
    }
}

/* Handle client messages, such as the WM_DELETE_WINDOW protocol */
void wm_handle_client_message(XClientMessageEvent *ev) {
    if (ev->message_type == wm.atoms[WM_PROTOCOLS] &&
        (Atom)ev->data.l[0] == wm.atoms[WM_DELETE_WINDOW]) {
        Client *c = client_find_by_window(ev->window);
        if (c) {
            client_close(c);
        }
    }
}

/* Remove a client when its window is destroyed */
void wm_handle_destroy_notify(XDestroyWindowEvent *ev) {
    client_remove(ev->window);
}

/* When the pointer enters a window, focus it */
void wm_handle_enter_notify(XCrossingEvent *ev) {
    if (ev->mode == NotifyNormal) {
        Client *c = client_find_by_window(ev->window);
        if (c) {
            client_focus(c);
        }
    }
}

/* Dispatch button press events to the input handler */
void wm_handle_button_press(XButtonEvent *ev) {
    input_handle_button((XEvent *)ev);
}

/* Dispatch key press events to the input handler */
void wm_handle_key_press(XKeyEvent *ev) {
    input_handle_key((XEvent *)ev);
}

/* Grab keys for global shortcuts */
void wm_grab_keys(void) {
    XUngrabKey(wm.display, AnyKey, AnyModifier, wm.root);
    unsigned int modifiers[] = { 0, wm.caps_lock_mask, wm.num_lock_mask,
                                 wm.caps_lock_mask | wm.num_lock_mask };
    KeyCode keycode;
    /* Alt+Tab */
    keycode = XKeysymToKeycode(wm.display, XK_Tab);
    for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++) {
        XGrabKey(wm.display, keycode, Mod1Mask | modifiers[i],
                 wm.root, True, GrabModeAsync, GrabModeAsync);
    }
    /* Alt+F4 */
    keycode = XKeysymToKeycode(wm.display, XK_F4);
    for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++) {
        XGrabKey(wm.display, keycode, Mod1Mask | modifiers[i],
                 wm.root, True, GrabModeAsync, GrabModeAsync);
    }
}

/* Grab buttons for window move/resize operations */
void wm_grab_buttons(void) {
    XUngrabButton(wm.display, AnyButton, AnyModifier, wm.root);
    unsigned int modifiers[] = { 0, wm.caps_lock_mask, wm.num_lock_mask,
                                 wm.caps_lock_mask | wm.num_lock_mask };
    /* Alt + Left click for moving windows */
    for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++) {
        XGrabButton(wm.display, Button1, Mod1Mask | modifiers[i],
                    wm.root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync, None, None);
    }
    /* Alt + Right click for resizing windows */
    for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++) {
        XGrabButton(wm.display, Button3, Mod1Mask | modifiers[i],
                    wm.root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync, None, None);
    }
}

/* Utility functions */

/* Retrieve an atom by name */
Atom wm_get_atom(const char *name) {
    return XInternAtom(wm.display, name, False);
}

/* Get a window property of a given type */
bool wm_get_window_prop(Window w, Atom prop, Atom type, int format,
                        unsigned char **data, unsigned long *items) {
    Atom actual_type;
    int actual_format;
    unsigned long bytes_after;
    if (XGetWindowProperty(wm.display, w, prop, 0, (~0L), False, type,
                           &actual_type, &actual_format, items, &bytes_after,
                           data) == Success && actual_type == type) {
        return true;
    }
    return false;
}

/* Set a window property */
void wm_set_window_prop(Window w, Atom prop, Atom type, int format,
                        unsigned char *data, int items) {
    XChangeProperty(wm.display, w, prop, type, format,
                    PropModeReplace, data, items);
}

/* Return the currently focused window */
Window wm_get_focused_window(void) {
    return wm.focused_window;
}

/* Determine if a window is mapped (viewable) */
bool wm_is_window_mapped(Window w) {
    XWindowAttributes attr;
    if (XGetWindowAttributes(wm.display, w, &attr)) {
        return attr.map_state == IsViewable;
    }
    return false;
}
