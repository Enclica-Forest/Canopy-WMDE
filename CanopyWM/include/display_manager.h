#ifndef CANOPY_DISPLAY_MANAGER_H
#define CANOPY_DISPLAY_MANAGER_H

#include <X11/extensions/Xrandr.h>
#include <stdbool.h>

typedef struct {
    XRROutputInfo *info;
    XRRCrtcInfo *crtc;
    int x, y;
    unsigned int width, height;
    int brightness;
} CanopyDisplay;

typedef struct {
    CanopyDisplay *displays;
    int num_displays;
    XRRScreenResources *resources;
} DisplayManager;

// Function declarations
void display_manager_init(void);
void display_manager_cleanup(void);
void display_update_all(void);
CanopyDisplay *display_get_at(int x, int y);
void display_set_brightness(CanopyDisplay *d, int brightness);

// External reference to the global display manager
extern DisplayManager display_manager;

#endif