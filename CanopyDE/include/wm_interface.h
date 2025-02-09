#ifndef CANOPY_WM_INTERFACE_H
#define CANOPY_WM_INTERFACE_H

#include <X11/Xlib.h>

// Atoms for WM-DE communication
#define CANOPY_ATOM_PREFIX "_CANOPY_"
#define CANOPY_WM_READY "_CANOPY_WM_READY"
#define CANOPY_DE_READY "_CANOPY_DE_READY"

// Check if CanopyWM is running
Bool check_wm_running(Display *display);

// Register with CanopyWM
void register_with_wm(Display *display);

#endif