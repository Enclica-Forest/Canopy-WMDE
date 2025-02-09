
// client.c
#include "client.h"
#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>

Client *client_create(Display *dpy, Window window, int x, int y, unsigned int width, unsigned int height) {
    Client *client = (Client *)malloc(sizeof(Client));
    if (!client) return NULL;

    client->window = window;
    client->frame = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), x, y, width, height, 0, 0, 0);
    client->x = x;
    client->y = y;
    client->width = width;
    client->height = height;
    client->is_fullscreen = false;
    client->is_floating = false;
    client->needs_redraw = false;
    client->title = NULL;
    client->next = NULL;

    return client;
}

void client_destroy(Display *dpy, Client *client) {
    if (client) {
        if (client->title) free(client->title);
        XDestroyWindow(dpy, client->frame);
        free(client);
    }
}

Client *client_from_window(Display *dpy, Window win) {
    // Placeholder: Replace with actual logic to retrieve client from window
    (void)dpy;
    (void)win;
    return NULL;
}

void client_set_title(Display *dpy, Client *client, const char *title) {
    if (client->title) free(client->title);
    client->title = strdup(title);
    XStoreName(dpy, client->window, title);
}

void client_focus(Display *dpy, Client *client) {
    XSetInputFocus(dpy, client->window, RevertToPointerRoot, CurrentTime);
}

void client_resize(Display *dpy, Client *client, unsigned int width, unsigned int height) {
    client->width = width;
    client->height = height;
    XResizeWindow(dpy, client->window, width, height);
}

void client_move(Display *dpy, Client *client, int x, int y) {
    client->x = x;
    client->y = y;
    XMoveWindow(dpy, client->window, x, y);
}
