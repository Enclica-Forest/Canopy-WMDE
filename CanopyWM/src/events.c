// src/events.c
#include "wm.h"
#include "client.h"
#include "input.h"
#include "notifications.h"
#include "display_manager.h"
#include "wallpaper.h"
#include "config.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

void handle_event(XEvent *ev) {
    if (!ev) return;

    switch (ev->type) {
        case MapRequest: {
            Window w = ev->xmaprequest.window;
            XWindowAttributes attr;
            
            // Get window attributes
            if (XGetWindowAttributes(wm.display, w, &attr)) {
                // Only manage windows we should manage
                if (!attr.override_redirect) {
                    XMapWindow(wm.display, w);
                    client_add(w);
                }
            }
            break;
        }

        case UnmapNotify: {
            // Only handle real unmap events
            if (ev->xunmap.event == wm.root) {
                client_remove(ev->xunmap.window);
            }
            break;
        }

        case ConfigureRequest: {
            XConfigureRequestEvent *conf = &ev->xconfigurerequest;
            XWindowChanges changes;
            
            // Copy requested changes
            changes.x = conf->x;
            changes.y = conf->y;
            changes.width = conf->width;
            changes.height = conf->height;
            changes.border_width = conf->border_width;
            changes.sibling = conf->above;
            changes.stack_mode = conf->detail;
            
            // Apply changes
            XConfigureWindow(wm.display, conf->window, conf->value_mask, &changes);
            break;
        }

        case KeyPress:
            input_handle_key(ev);
            break;

        case ButtonPress:
            input_handle_button(ev);
            break;

        case MotionNotify:
            // Only handle motion if we're dragging
            if (input_manager.mouse_dragging) {
                input_handle_motion(ev);
            }
            break;

        case ButtonRelease:
            if (ev->xbutton.button == Button1) {
                input_manager.mouse_dragging = false;
            }
            break;

        case PropertyNotify: {
            if (ev->xproperty.atom == XA_WM_NAME ||
                ev->xproperty.atom == wm.atoms[NET_WM_NAME]) {
                Client *c = client_find_by_window(ev->xproperty.window);
                if (c) {
                    client_update_title(c);
                }
            }
            break;
        }

        case ClientMessage: {
            XClientMessageEvent *cm = &ev->xclient;
            if (cm->message_type == wm.atoms[WM_PROTOCOLS]) {
                if ((Atom)cm->data.l[0] == wm.atoms[WM_DELETE_WINDOW]) {
                    Client *c = client_find_by_window(cm->window);
                    if (c) {
                        client_close(c);
                    }
                }
            }
            break;
        }

        case DestroyNotify:
            client_remove(ev->xdestroywindow.window);
            break;

        case EnterNotify: {
            if (ev->xcrossing.mode == NotifyNormal) {
                Client *c = client_find_by_window(ev->xcrossing.window);
                if (c) {
                    client_focus(c);
                }
            }
            break;
        }

        case FocusIn: {
            Client *c = client_find_by_window(ev->xfocus.window);
            if (c) {
                client_manager.focused = c;
            }
            break;
        }

        case RRScreenChangeNotify:
            if (ev->type - wm.randr_event_base == RRScreenChangeNotify) {
                display_update_all();
                wallpaper_render();
            }
            break;

        case ConfigureNotify:
            // Handle window configuration changes
            // This is usually for windows we don't manage
            break;

        case ReparentNotify:
            // Handle window reparenting if needed
            break;
    }
}