// CanopyWM/include/canopywm.h
#ifndef CANOPY_WM_H
#define CANOPY_WM_H

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <libnotify/notify.h>
#include <alsa/asoundlib.h>
#include <systemd/sd-bus.h>

#define MAX_DISPLAYS 4
#define MAX_NOTIFICATIONS 32

typedef struct {
    Window window;
    int x, y;
    unsigned int width, height;
    char *title;
    Bool isFullscreen;
    Bool isMinimized;
} Client;

typedef struct {
    XRROutputInfo *info;
    XRRCrtcInfo *crtc;
    int x, y;
    unsigned int width, height;
} Display;

typedef struct {
    NotifyNotification *notification;
    char *summary;
    char *body;
    int timeout;
} NotificationEntry;

typedef struct {
    Display *display;
    Window root;
    Client *clients;
    int num_clients;
    int screen;
    GC gc;
    cairo_surface_t *surface;
    cairo_t *cr;
    
    // Multi-display support
    Display displays[MAX_DISPLAYS];
    int num_displays;
    XRRScreenResources *resources;
    
    // Audio control
    snd_mixer_t *mixer;
    snd_mixer_elem_t *volume_elem;
    
    // Notification system
    NotificationEntry notifications[MAX_NOTIFICATIONS];
    int notification_count;
    
    // Wallpaper
    cairo_surface_t *wallpaper;
    char *wallpaper_path;
    
    // Input handling
    XIC xic;
    XIM xim;
    
    // System bus connection
    sd_bus *bus;
} WM;

// Window management
void init_wm();
void cleanup_wm();
void add_client(Window w);
void remove_client(Window w);
void toggle_fullscreen(Client *c);
void minimize_window(Client *c);
void maximize_window(Client *c);
void move_window(Client *c, int x, int y);
void resize_window(Client *c, int width, int height);

// Display management
void init_displays();
void update_displays();
Display *get_display_at(int x, int y);
void set_display_brightness(Display *d, int brightness);

// Audio control
void init_audio();
void set_volume(int volume);
void toggle_mute();
int get_volume();
Bool is_muted();

// Wallpaper management
void set_wallpaper(const char *path);
void render_wallpaper();

// Input handling
void init_input();
void handle_key_event(XEvent *ev);
void handle_mouse_event(XEvent *ev);

// Notification system
void init_notifications();
void show_notification(const char *summary, const char *body, int timeout);
void clear_notifications();

// Event handling
void handle_event(XEvent *ev);

#endif