// client.h
#ifndef CLIENT_H
#define CLIENT_H

#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct Decoration {
    Window handle;
    Window close_btn;
    Window max_btn;
    Window min_btn;
    unsigned int button_size;
} Decoration;

typedef struct Client {
    Window window;
    Window frame;
    char *title;
    int x, y;
    unsigned int width, height;
    bool is_fullscreen;
    bool is_floating;
    bool needs_redraw;
    struct Client *next;
    Decoration decor;
} Client;

// Client management API
Client *client_create(Display *dpy, Window window, int x, int y,
                     unsigned int width, unsigned int height);
void client_destroy(Display *dpy, Client *client);
Client *client_from_window(Display *dpy, Window win);
void client_set_title(Display *dpy, Client *client, const char *title);
void client_focus(Display *dpy, Client *client);
void client_resize(Display *dpy, Client *client, 
                  unsigned int width, unsigned int height);
void client_move(Display *dpy, Client *client, int x, int y);

#endif // CLIENT_H