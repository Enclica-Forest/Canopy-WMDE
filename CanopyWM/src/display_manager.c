#include "display_manager.h"
#include "wm.h"
#include <stdlib.h>
#include <systemd/sd-bus.h>

// Global display manager instance
DisplayManager display_manager;

void display_manager_init(void) {
    display_manager.resources = XRRGetScreenResources(wm.display, wm.root);
    display_manager.displays = malloc(sizeof(CanopyDisplay) * 16);
    display_manager.num_displays = 0;
    
    display_update_all();
}

void display_manager_cleanup(void) {
    if (display_manager.displays) {
        for (int i = 0; i < display_manager.num_displays; i++) {
            if (display_manager.displays[i].info) {
                XRRFreeOutputInfo(display_manager.displays[i].info);
            }
            if (display_manager.displays[i].crtc) {
                XRRFreeCrtcInfo(display_manager.displays[i].crtc);
            }
        }
        free(display_manager.displays);
    }
    
    if (display_manager.resources) {
        XRRFreeScreenResources(display_manager.resources);
    }
}

void display_update_all(void) {
    // First, free existing display info
    for (int i = 0; i < display_manager.num_displays; i++) {
        if (display_manager.displays[i].info) {
            XRRFreeOutputInfo(display_manager.displays[i].info);
        }
        if (display_manager.displays[i].crtc) {
            XRRFreeCrtcInfo(display_manager.displays[i].crtc);
        }
    }
    
    display_manager.num_displays = 0;
    
    // Update with current display information
    for (int i = 0; i < display_manager.resources->noutput; i++) {
        XRROutputInfo *output_info = XRRGetOutputInfo(wm.display, 
                                                    display_manager.resources,
                                                    display_manager.resources->outputs[i]);
        
        if (output_info->connection == RR_Connected) {
            CanopyDisplay *d = &display_manager.displays[display_manager.num_displays];
            d->info = output_info;
            d->crtc = XRRGetCrtcInfo(wm.display, display_manager.resources,
                                    output_info->crtc);
            d->x = d->crtc->x;
            d->y = d->crtc->y;
            d->width = d->crtc->width;
            d->height = d->crtc->height;
            d->brightness = 100; // Default brightness
            display_manager.num_displays++;
        } else {
            XRRFreeOutputInfo(output_info);
        }
    }
}

CanopyDisplay *display_get_at(int x, int y) {
    for (int i = 0; i < display_manager.num_displays; i++) {
        CanopyDisplay *d = &display_manager.displays[i];
        if (x >= d->x && x < d->x + d->width &&
            y >= d->y && y < d->y + d->height) {
            return d;
        }
    }
    return NULL;
}

void display_set_brightness(CanopyDisplay *d, int brightness) {
    if (!d || !d->info) return;
    
    // Clamp brightness value
    if (brightness < 0) brightness = 0;
    if (brightness > 100) brightness = 100;
    
    d->brightness = brightness;
    
    // Use systemd-logind to set actual brightness
    sd_bus_message *m = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    
    sd_bus_call_method(wm.bus,
                      "org.freedesktop.login1",
                      "/org/freedesktop/login1/session/auto",
                      "org.freedesktop.login1.Session",
                      "SetBrightness",
                      &error,
                      &m,
                      "ssu",
                      "backlight",
                      d->info->name,
                      brightness);
}
