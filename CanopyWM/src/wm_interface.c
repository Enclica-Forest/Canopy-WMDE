#include "wm_interface.h"
#include "wm.h"
#include <X11/Xatom.h>
#include <stdio.h>

static struct {
    Atom wm_ready;
    Atom de_ready;
    bool de_is_ready;
    Window root;
} interface = {0};

void wm_interface_init(Display *display) {
    interface.wm_ready = XInternAtom(display, CANOPY_WM_READY, False);
    interface.de_ready = XInternAtom(display, CANOPY_DE_READY, False);
    interface.de_is_ready = false;
    interface.root = DefaultRootWindow(display);

    // Announce WM is ready
    XChangeProperty(display, interface.root,
                   interface.wm_ready, XA_WINDOW, 32,
                   PropModeReplace, (unsigned char *)&interface.root, 1);
}

void wm_interface_cleanup(void) {
    if (wm.display) {
        XDeleteProperty(wm.display, interface.root, interface.wm_ready);
    }
}

void wm_interface_handle_message(XEvent *ev) {
    if (ev->type == ClientMessage) {
        XClientMessageEvent *cm = &ev->xclient;
        
        if (cm->message_type == interface.de_ready) {
            interface.de_is_ready = true;
            printf("CanopyDE has connected\n");
            
            // Acknowledge DE registration
            XEvent reply;
            reply.type = ClientMessage;
            reply.xclient.window = interface.root;
            reply.xclient.message_type = interface.wm_ready;
            reply.xclient.format = 32;
            reply.xclient.data.l[0] = 1; // Acknowledgment code
            
            XSendEvent(wm.display, interface.root, False,
                      SubstructureNotifyMask | SubstructureRedirectMask,
                      &reply);
            XFlush(wm.display);
        }
    }
}

bool wm_interface_is_de_ready(void) {
    return interface.de_is_ready;
}