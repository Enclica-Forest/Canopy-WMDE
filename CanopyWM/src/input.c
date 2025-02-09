
// src/input.c
#include "input.h"
#include "wm.h"
#include "client.h"
#include <stdlib.h>
#include <string.h>

// Global input manager instance
InputManager input_manager;

void input_manager_init(void) {
    input_manager.capacity = 32;
    input_manager.keybinds = malloc(sizeof(Keybind) * input_manager.capacity);
    input_manager.num_keybinds = 0;
    input_manager.mouse_dragging = false;
    
    // Initialize XIM
    input_manager.xim = XOpenIM(wm.display, NULL, NULL, NULL);
    if (input_manager.xim) {
        input_manager.xic = XCreateIC(input_manager.xim,
                                    XNInputStyle,
                                    XIMPreeditNothing | XIMStatusNothing,
                                    XNClientWindow, wm.root,
                                    XNFocusWindow, wm.root,
                                    NULL);
    }
    
    // Register default keybindings
    input_register_keybind(XK_F11, Mod1Mask, client_focus_next);     // Alt+F11
    input_register_keybind(XK_Tab, Mod1Mask, client_cycle_focus);    // Alt+Tab
    input_register_keybind(XK_q, Mod1Mask | ShiftMask, client_close_focused); // Alt+Shift+Q
    input_register_keybind(XK_f, Mod1Mask, client_toggle_fullscreen_focused); // Alt+F
}

void input_manager_cleanup(void) {
    if (input_manager.keybinds) {
        free(input_manager.keybinds);
        input_manager.keybinds = NULL;
    }
    
    if (input_manager.xic) {
        XDestroyIC(input_manager.xic);
    }
    
    if (input_manager.xim) {
        XCloseIM(input_manager.xim);
    }
}

void input_handle_key(XEvent *ev) {
    XKeyEvent *key_ev = &ev->xkey;
    KeySym keysym = XkbKeycodeToKeysym(wm.display, key_ev->keycode, 0, 0);
    
    for (int i = 0; i < input_manager.num_keybinds; i++) {
        if (input_manager.keybinds[i].key == keysym &&
            input_manager.keybinds[i].modifiers == key_ev->state) {
            input_manager.keybinds[i].callback();
            break;
        }
    }
}

void input_handle_button(XEvent *ev) {
    XButtonEvent *button_ev = &ev->xbutton;
    
    if (button_ev->subwindow == None) return;
    
    if (button_ev->button == Button1 && button_ev->state & Mod1Mask) {
        input_manager.mouse_dragging = true;
        input_manager.drag_window = button_ev->subwindow;
        input_manager.drag_start_x = button_ev->x_root;
        input_manager.drag_start_y = button_ev->y_root;
        XRaiseWindow(wm.display, button_ev->subwindow);
    }
}

void input_handle_motion(XEvent *ev) {
    if (!input_manager.mouse_dragging) return;
    
    XMotionEvent *motion = &ev->xmotion;
    
    int dx = motion->x_root - input_manager.drag_start_x;
    int dy = motion->y_root - input_manager.drag_start_y;
    
    Client *c = client_find_by_window(input_manager.drag_window);
    if (c) {
        client_move(c, c->x + dx, c->y + dy);
        input_manager.drag_start_x = motion->x_root;
        input_manager.drag_start_y = motion->y_root;
    }
}

void input_register_keybind(KeySym key, unsigned int modifiers, void (*callback)(void)) {
    if (input_manager.num_keybinds >= input_manager.capacity) {
        input_manager.capacity *= 2;
        input_manager.keybinds = realloc(input_manager.keybinds,
                                       sizeof(Keybind) * input_manager.capacity);
    }
    
    input_manager.keybinds[input_manager.num_keybinds].key = key;
    input_manager.keybinds[input_manager.num_keybinds].modifiers = modifiers;
    input_manager.keybinds[input_manager.num_keybinds].callback = callback;
    input_manager.num_keybinds++;
    
    XGrabKey(wm.display,
            XKeysymToKeycode(wm.display, key),
            modifiers,
            wm.root,
            True,
            GrabModeAsync,
            GrabModeAsync);
}
