#include "wm_interface.h"
#include <X11/Xatom.h>
#include <stdio.h>

Bool check_wm_running(Display *display) {
    Atom wm_ready = XInternAtom(display, CANOPY_WM_READY, False);
    Window root = DefaultRootWindow(display);
    
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;
    
    if (XGetWindowProperty(display, root, wm_ready, 0, 1, False,
                          XA_WINDOW, &actual_type, &actual_format,
                          &nitems, &bytes_after, &data) == Success) {
        if (data) {
            XFree(data);
            return True;
        }
    }
    return False;
}

void register_with_wm(Display *display) {
    Atom de_ready = XInternAtom(display, CANOPY_DE_READY, False);
    Window root = DefaultRootWindow(display);
    
    // Send registration message
    XEvent ev;
    ev.type = ClientMessage;
    ev.xclient.window = root;
    ev.xclient.message_type = de_ready;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = 1; // Registration code
    
    XSendEvent(display, root, False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &ev);
    XFlush(display);
}