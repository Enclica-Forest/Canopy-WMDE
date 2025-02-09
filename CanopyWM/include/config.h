// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// Configuration structures
typedef struct {
    unsigned int border_width;
    unsigned int border_color;
    unsigned int focus_border_color;
    unsigned int titlebar_height;
    unsigned int titlebar_color;
    unsigned int title_text_color;
    unsigned int button_color;
    unsigned int button_text_color;
    unsigned int font_size;
} WindowConfig;

typedef struct {
    unsigned int master_size;
    unsigned int master_count;
    float split_ratio;
    unsigned int snap_distance;
} LayoutConfig;

typedef enum {
    WALLPAPER_STRETCH,
    WALLPAPER_CENTER,
    WALLPAPER_TILE
} WallpaperMode;

typedef struct {
    char *wallpaper_path;
    WallpaperMode wallpaper_mode;
    unsigned int background_color;
} AppearanceConfig;

typedef struct {
    unsigned int volume_step;
    unsigned int brightness_step;
} SystemConfig;

typedef struct {
    WindowConfig window;
    LayoutConfig layout;
    AppearanceConfig appearance;
    SystemConfig system;
} Config;

extern Config config;

// Function declarations
void config_init(void);
void config_cleanup(void);
bool config_load(const char *path);

#endif // CONFIG_H