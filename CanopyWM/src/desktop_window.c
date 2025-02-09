#include "desktop_window.h"
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdio.h>
#include <cairo/cairo-xlib.h>  // Needed for cairo_xlib_surface_create

Window create_desktop_window(Display *display, int screen, Window root,
                             int *width, int *height,
                             cairo_surface_t **surface, cairo_t **cr) {
    // Get screen dimensions.
    *width = DisplayWidth(display, screen);
    *height = DisplayHeight(display, screen);
    
    // Set window attributes: override_redirect so it isn’t managed,
    // and a black background.
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.background_pixel = BlackPixel(display, screen);
    attrs.event_mask = ExposureMask;
    
    // Create a window that covers the entire screen.
    Window desktop = XCreateWindow(display, root,
                                   0, 0, *width, *height,
                                   0, CopyFromParent, InputOutput,
                                   CopyFromParent,
                                   CWOverrideRedirect | CWBackPixel | CWEventMask,
                                   &attrs);
    if (!desktop) {
        fprintf(stderr, "Failed to create desktop window\n");
        return 0;
    }
    
    // Set the window type to DESKTOP (if your WM supports this).
    Atom window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom desktop_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    XChangeProperty(display, desktop, window_type, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&desktop_type, 1);
    
    // Map the window and lower it.
    XMapWindow(display, desktop);
    XLowerWindow(display, desktop);
    XFlush(display);
    
    // Create a Cairo surface and context for this window.
    *surface = cairo_xlib_surface_create(display, desktop,
                                         DefaultVisual(display, screen),
                                         *width, *height);
    *cr = cairo_create(*surface);
    
    return desktop;
}

void render_wallpaper_on_desktop(Display *display, int screen, Window desktop,
                                 cairo_surface_t *desktop_surface, cairo_t *desktop_cr,
                                 cairo_surface_t *wallpaper_surface) {
    if (!desktop_cr || !desktop_surface || !wallpaper_surface)
        return;
    
    int width = DisplayWidth(display, screen);
    int height = DisplayHeight(display, screen);
    int img_width = cairo_image_surface_get_width(wallpaper_surface);
    int img_height = cairo_image_surface_get_height(wallpaper_surface);
    
    // Clear the desktop window.
    cairo_set_source_rgb(desktop_cr, 0, 0, 0);
    cairo_paint(desktop_cr);
    
    // For example, use stretch mode:
    cairo_save(desktop_cr);
    cairo_scale(desktop_cr, (double)width / img_width, (double)height / img_height);
    cairo_set_source_surface(desktop_cr, wallpaper_surface, 0, 0);
    cairo_paint(desktop_cr);
    cairo_restore(desktop_cr);
    
    cairo_surface_flush(desktop_surface);
    XFlush(display);
}
