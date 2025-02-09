#ifndef CANOPY_WM_H
#define CANOPY_WM_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <X11/XKBlib.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <systemd/sd-bus.h>
#include <stdbool.h>

// Atom indices
enum {
    WM_PROTOCOLS,
    WM_DELETE_WINDOW,
    WM_STATE,
    WM_TAKE_FOCUS,
    NET_WM_NAME,
    NET_WM_STATE,
    NET_WM_STATE_FULLSCREEN,
    NET_ACTIVE_WINDOW,
    NET_WM_WINDOW_TYPE,
    NET_WM_WINDOW_TYPE_DOCK,
    NET_WM_WINDOW_TYPE_TOOLBAR,
    NET_WM_WINDOW_TYPE_MENU,
    NET_WM_WINDOW_TYPE_UTILITY,
    NET_WM_WINDOW_TYPE_SPLASH,
    NET_WM_WINDOW_TYPE_DIALOG,
    NET_WM_WINDOW_TYPE_NORMAL,
    ATOM_COUNT
};

// Window Manager state structure
typedef struct {
    Display *display;          // X11 Display connection
    Window root;               // Root window
    int screen;                // Default screen
    GC gc;                     // Graphics context
    cairo_surface_t *surface;  // Cairo surface for WM drawing
    cairo_t *cr;               // Cairo context for WM drawing
    sd_bus *bus;               // systemd bus connection
    Atom atoms[ATOM_COUNT];    // Common atoms
    bool running;              // Main loop control
    int randr_event_base;      // RandR extension event base
    int randr_error_base;      // RandR extension error base
    unsigned int num_lock_mask;    // Numlock mask
    unsigned int scroll_lock_mask; // Scroll lock mask
    unsigned int caps_lock_mask;   // Caps lock mask
    Window focused_window;     // Currently focused window

    // Desktop background window (for wallpaper)
    Window desktop_window;
    cairo_surface_t *desktop_surface;
    cairo_t *desktop_cr;
    int desktop_width;
    int desktop_height;

    // Window dragging state
    bool dragging;             // Whether we're currently dragging a window
    int drag_start_x;          // Initial cursor X position for dragging
    int drag_start_y;          // Initial cursor Y position for dragging
} WM;

// Global window manager instance
extern WM wm;

// Core functions
void wm_init(void);
void wm_cleanup(void);
void wm_run(void);

// Event handling
void wm_handle_event(XEvent *ev);
void wm_handle_map_request(XMapRequestEvent *ev);
void wm_handle_configure_request(XConfigureRequestEvent *ev);
void wm_handle_property_notify(XPropertyEvent *ev);
void wm_handle_client_message(XClientMessageEvent *ev);
void wm_handle_destroy_notify(XDestroyWindowEvent *ev);
void wm_handle_enter_notify(XCrossingEvent *ev);
void wm_handle_button_press(XButtonEvent *ev);
void wm_handle_key_press(XKeyEvent *ev);

// Window management (the prototypes below can be expanded as needed)
void wm_frame_window(Window w);
void wm_unframe_window(Window w);
void wm_focus_window(Window w);
void wm_move_window(Window w, int x, int y);
void wm_resize_window(Window w, unsigned int width, unsigned int height);
void wm_maximize_window(Window w);
void wm_minimize_window(Window w);
void wm_close_window(Window w);

// Utility functions
void wm_grab_keys(void);
void wm_grab_buttons(void);
Atom wm_get_atom(const char *name);
bool wm_get_window_prop(Window w, Atom prop, Atom type, int format, 
                          unsigned char **data, unsigned long *items);
void wm_set_window_prop(Window w, Atom prop, Atom type, int format, 
                          unsigned char *data, int items);
Window wm_get_focused_window(void);
bool wm_is_window_mapped(Window w);

#endif
