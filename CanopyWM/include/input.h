#ifndef CANOPY_INPUT_H
#define CANOPY_INPUT_H

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>

typedef struct {
    KeySym key;
    unsigned int modifiers;
    void (*callback)(void);
} Keybind;

typedef struct {
    Keybind *keybinds;
    int num_keybinds;
    int capacity;
    XIC xic;
    XIM xim;
    bool mouse_dragging;
    Window drag_window;
    int drag_start_x;
    int drag_start_y;
} InputManager;

// Function declarations
void input_manager_init(void);
void input_manager_cleanup(void);
void input_handle_key(XEvent *ev);
void input_handle_button(XEvent *ev);
void input_handle_motion(XEvent *ev);
void input_register_keybind(KeySym key, unsigned int modifiers, void (*callback)(void));

// Global input manager instance
extern InputManager input_manager;

#endif
    