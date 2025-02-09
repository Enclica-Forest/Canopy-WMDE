#ifndef DESKTOP_WINDOW_H
#define DESKTOP_WINDOW_H

#include <X11/Xlib.h>
#include <cairo/cairo.h>

/* 
 * Creates a full-screen, override-redirect desktop window.
 * Parameters:
 *   display   - The X Display.
 *   screen    - The screen number.
 *   root      - The root window.
 *   width     - Pointer to an integer to receive the width.
 *   height    - Pointer to an integer to receive the height.
 *   surface   - Pointer to a cairo_surface_t* to receive the Cairo surface.
 *   cr        - Pointer to a cairo_t* to receive the Cairo context.
 *
 * Returns:
 *   The created desktop window (or 0 on failure).
 */
Window create_desktop_window(Display *display, int screen, Window root,
                             int *width, int *height,
                             cairo_surface_t **surface, cairo_t **cr);

/*
 * Renders the given wallpaper (Cairo surface) onto the desktop window.
 */
void render_wallpaper_on_desktop(Display *display, int screen, Window desktop,
                                 cairo_surface_t *desktop_surface, cairo_t *desktop_cr,
                                 cairo_surface_t *wallpaper_surface);

#endif
