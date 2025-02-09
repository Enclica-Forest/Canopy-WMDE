// main.c
#include "wm.h"
#include "client.h"
#include "display_manager.h"
#include "audio.h"
#include "input.h"
#include "notifications.h"
#include "config.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <math.h>
#include <time.h>

// Global state
static volatile sig_atomic_t running = 1;
static pid_t xephyr_pid = -1;
static pid_t de_pid = -1;

static struct {
    cairo_surface_t *surface;
    cairo_t *cr;
    double angle;
    int counter;
    bool active;
    double fade_alpha;
    bool de_loaded;
    int animation_stage;
} loading = {0};

static const char *default_de_paths[] = {
    "/usr/local/bin/canopy-de",
    "/usr/bin/canopy-de",
    "./canopy-de",
    "../CanopyDE/build/canopy-de",
    NULL
};

// Function declarations
static void signal_handler(int signum);
static void setup_signals(void);
static void cleanup_children(void);
static const char* find_desktop_environment(void);
static bool start_desktop_environment(const char *de_path);
static bool start_debug_session(void);
static void update_loading_animation(void);
static void start_loading_animation(void);
static void stop_loading_animation(void);
static void print_usage(const char *program_name);
void client_manager_init(Display *dpy);
void client_manager_cleanup(Display *dpy);

static void signal_handler(int signum) {
    (void)signum;
    running = 0;
    LOG_INFO("Signal received, shutting down.");
}

static void setup_signals(void) {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    LOG_INFO("Signals set up.");
}

static void cleanup_children(void) {
    LOG_INFO("Cleaning up child processes.");
    if (de_pid > 0) {
        kill(de_pid, SIGTERM);
        waitpid(de_pid, NULL, 0);
        LOG_INFO("Desktop environment process %d terminated.", de_pid);
    }
    if (xephyr_pid > 0) {
        kill(xephyr_pid, SIGTERM);
        waitpid(xephyr_pid, NULL, 0);
        LOG_INFO("Xephyr process %d terminated.", xephyr_pid);

        const char *display = getenv("DISPLAY");
        if (display && display[0] == ':') {
            char lockfile[256];
            snprintf(lockfile, sizeof(lockfile), "/tmp/.X%s-lock", display + 1);
            unlink(lockfile);
            LOG_INFO("Removed lockfile %s.", lockfile);
        }
    }
}

static void start_loading_animation(void) {
    int width = DisplayWidth(wm.display, wm.screen);
    int height = DisplayHeight(wm.display, wm.screen);

    loading.surface = cairo_xlib_surface_create(
        wm.display, wm.root,
        DefaultVisual(wm.display, wm.screen),
        width, height
    );

    loading.cr = cairo_create(loading.surface);
    loading.angle = 0;
    loading.counter = 0;
    loading.active = true;
    loading.fade_alpha = 1.0;
    loading.de_loaded = false;
    LOG_INFO("Started loading animation.");
}

static void update_loading_animation(void) {
    if (!loading.active) return;

    int width = DisplayWidth(wm.display, wm.screen);
    int height = DisplayHeight(wm.display, wm.screen);

    cairo_set_operator(loading.cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(loading.cr);
    cairo_set_operator(loading.cr, CAIRO_OPERATOR_OVER);

    for (int i = 0; i < 5; i++) {
        double t = loading.counter * 0.01 + i * M_PI / 2.5;
        cairo_set_source_rgba(loading.cr, 0.2, 0.4, 0.8, 0.1);
        cairo_arc(loading.cr,
                  width/2 + cos(t) * 100,
                  height/2 + sin(t) * 100,
                  150 + sin(loading.counter * 0.05) * 50,
                  0, 2 * M_PI);
        cairo_fill(loading.cr);
    }

    if (!loading.de_loaded) {
        cairo_translate(loading.cr, width/2, height/2);
        cairo_rotate(loading.cr, loading.angle);

        for (int i = 0; i < 12; i++) {
            double alpha = 1.0 - (i * 0.08);
            double size = 3 + sin(loading.counter * 0.1 + i) * 2;
            cairo_set_source_rgba(loading.cr, 1.0, 1.0, 1.0, alpha * loading.fade_alpha);
            cairo_arc(loading.cr, 0, -30, size, 0, 2 * M_PI);
            cairo_fill(loading.cr);
            cairo_rotate(loading.cr, M_PI / 6);
        }

        cairo_identity_matrix(loading.cr);
        cairo_select_font_face(loading.cr, "Sans", 
                             CAIRO_FONT_SLANT_NORMAL,
                             CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(loading.cr, 24 + sin(loading.counter * 0.1) * 2);
        cairo_set_source_rgba(loading.cr, 1.0, 1.0, 1.0, loading.fade_alpha);

        const char *text = "Starting Canopy Desktop...";
        cairo_text_extents_t extents;
        cairo_text_extents(loading.cr, text, &extents);

        cairo_move_to(loading.cr,
                     (width - extents.width) / 2,
                     height/2 + 60);
        cairo_show_text(loading.cr, text);
    } else {
        loading.fade_alpha -= 0.02;
        if (loading.fade_alpha <= 0) {
            loading.active = false;
            LOG_INFO("Loading animation faded out.");
        }
    }

    loading.angle += 0.1;
    loading.counter++;

    cairo_surface_flush(loading.surface);
    XFlush(wm.display);
}

static void stop_loading_animation(void) {
    if (loading.cr) cairo_destroy(loading.cr);
    if (loading.surface) cairo_surface_destroy(loading.surface);
    loading.active = false;
    XClearWindow(wm.display, wm.root);
    XFlush(wm.display);
    LOG_INFO("Stopped loading animation.");
}

static const char* find_desktop_environment(void) {
    for (const char **path = default_de_paths; *path != NULL; path++) {
        if (access(*path, X_OK) == 0) {
            LOG_INFO("Found desktop environment executable: %s", *path);
            return *path;
        }
    }
    LOG_WARN("No desktop environment executable found in default paths.");
    return NULL;
}

static bool start_desktop_environment(const char *de_path) {
    LOG_INFO("Starting desktop environment: %s", de_path);
    de_pid = fork();
    if (de_pid == 0) {
        execl(de_path, de_path, NULL);
        LOG_ERROR("Failed to start desktop environment (%s).", de_path);
        perror("execl");
        exit(1);
    } else if (de_pid < 0) {
        LOG_ERROR("Fork failed when starting desktop environment.");
        perror("fork");
        return false;
    }
    return true;
}

static bool start_debug_session(void) {
    LOG_INFO("Starting debug session with Xephyr.");
    for (int i = 0; i < 10; i++) {
        char lockfile[32];
        snprintf(lockfile, sizeof(lockfile), "/tmp/.X%d-lock", i);
        unlink(lockfile);
    }

    for (int display_num = 1; display_num < 10; display_num++) {
        char display_str[16];
        snprintf(display_str, sizeof(display_str), ":%d", display_num);

        Display *test = XOpenDisplay(display_str);
        if (test) {
            XCloseDisplay(test);
            continue;
        }

        xephyr_pid = fork();
        if (xephyr_pid == 0) {
            char display_arg[16];
            snprintf(display_arg, sizeof(display_arg), ":%d", display_num);

            execlp("Xephyr", "Xephyr", 
                   display_arg,
                   "-ac",
                   "-screen", "1920x1080",
                   "-resizeable",
                   "-title", "CanopyWM [Debug Mode]",
                   NULL);
            LOG_ERROR("Failed to start Xephyr.");
            perror("execlp");
            exit(1);
        } else if (xephyr_pid < 0) {
            LOG_ERROR("Fork failed when starting Xephyr.");
            perror("fork");
            return false;
        }

        sleep(1);
        setenv("DISPLAY", display_str, 1);
        LOG_INFO("Debug session started on display %s.", display_str);
        return true;
    }

    LOG_ERROR("Could not find an available display for debug session.");
    return false;
}

static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Options:\n");
    printf("  --debug            Run in debug mode with Xephyr\n");
    printf("  --de-path PATH     Specify path to desktop environment executable\n");
    printf("  --help             Show this help message\n");
}

int main(int argc, char *argv[]) {
    bool debug_mode = false;
    const char *de_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
        }
        else if (strcmp(argv[i], "--de-path") == 0 && i + 1 < argc) {
            de_path = argv[++i];
        }
        else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    if (de_path == NULL) {
        de_path = find_desktop_environment();
        if (de_path == NULL) {
            LOG_ERROR("Could not find desktop environment executable. Use --de-path option.");
            return 1;
        }
    }

    if (debug_mode && !start_debug_session()) {
        LOG_ERROR("Failed to start debug session.");
        return 1;
    }

    LOG_INFO("Initializing subsystems...");
    wm_init();
    config_init();
    client_manager_init(wm.display);
    display_manager_init();
    audio_manager_init();
    input_manager_init();
    notification_manager_init();

    setup_signals();
    atexit(cleanup_children);

    start_loading_animation();

    if (!start_desktop_environment(de_path)) {
        LOG_ERROR("Failed to start desktop environment.");
        stop_loading_animation();
        return 1;
    }

    LOG_INFO("CanopyWM initialized and running.");
    time_t start_time = time(NULL);

    while (running) {
        while (XPending(wm.display)) {
            XEvent ev;
            XNextEvent(wm.display, &ev);

            wm_handle_event(&ev);
        }

        if (!loading.de_loaded && difftime(time(NULL), start_time) >= 5) {
            loading.de_loaded = true;
            LOG_INFO("Desktop environment marked as loaded.");
        }

        if (loading.active) update_loading_animation();

        notification_clear_expired();

        int status;
        if (waitpid(de_pid, &status, WNOHANG) == de_pid) {
            LOG_INFO("Desktop environment exited, shutting down.");
            running = 0;
            break;
        }

        struct timeval tv = {0, 16666};
        int xfd = ConnectionNumber(wm.display);
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(xfd, &fds);
        select(xfd + 1, &fds, NULL, NULL, &tv);
    }

    LOG_INFO("Shutting down CanopyWM...");
    stop_loading_animation();
    notification_manager_cleanup();
    input_manager_cleanup();
    audio_manager_cleanup();
    display_manager_cleanup();
    client_manager_cleanup(wm.display);
    config_cleanup();
    wm_cleanup();

    LOG_INFO("Shutdown complete.");
    return 0;
}
