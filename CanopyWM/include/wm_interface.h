#ifndef CANOPY_WM_INTERFACE_H
#define CANOPY_WM_INTERFACE_H

#include <X11/Xlib.h>
#include <stdbool.h>

// Communication atoms
#define CANOPY_ATOM_PREFIX "_CANOPY_"
#define CANOPY_WM_READY "_CANOPY_WM_READY"
#define CANOPY_DE_READY "_CANOPY_DE_READY"

void wm_interface_init(Display *display);
void wm_interface_cleanup(void);
void wm_interface_handle_message(XEvent *ev);
bool wm_interface_is_de_ready(void);

#endif
