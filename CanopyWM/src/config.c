
// config.c
#include "config.h"
#include <stdlib.h>
#include <string.h>

Config config;

void config_init(void) {
    // Window defaults
    config.window.border_width = 2;
    config.window.border_color = 0x333333;
    config.window.focus_border_color = 0x007acc;
    config.window.titlebar_height = 24;
    config.window.titlebar_color = 0x202020;
    config.window.title_text_color = 0xffffff;
    config.window.button_color = 0x404040;
    config.window.button_text_color = 0xffffff;
    config.window.font_size = 12;

    // Layout defaults
    config.layout.master_size = 600;
    config.layout.master_count = 1;
    config.layout.split_ratio = 0.6f;
    config.layout.snap_distance = 10;

    // Appearance defaults
    config.appearance.wallpaper_path = strdup("default_wallpaper.jpg");
    config.appearance.wallpaper_mode = WALLPAPER_STRETCH;
    config.appearance.background_color = 0xcccccc;

    // System defaults
    config.system.volume_step = 5;
    config.system.brightness_step = 5;
}

void config_cleanup(void) {
    free(config.appearance.wallpaper_path);
    config.appearance.wallpaper_path = NULL;
}

bool config_load(const char *path) {
    // Implementation would go here
    (void)path; // Temporary unused parameter
    return true;
}
