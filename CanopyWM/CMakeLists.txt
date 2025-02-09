cmake_minimum_required(VERSION 3.10)
project(CanopyWM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XRANDR REQUIRED xrandr)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(XCURSOR REQUIRED xcursor)  # Added for Xcursor support

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/CanopyWM/include
    ${GLIB2_INCLUDE_DIRS}
    ${LIBNOTIFY_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    ${XRANDR_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${SYSTEMD_INCLUDE_DIRS}
    ${XCURSOR_INCLUDE_DIRS}  # Added for Xcursor support
)

# Link directories
link_directories(
    ${GLIB2_LIBRARY_DIRS}
    ${LIBNOTIFY_LIBRARY_DIRS}
    ${ALSA_LIBRARY_DIRS}
    ${X11_LIBRARY_DIRS}
    ${XRANDR_LIBRARY_DIRS}
    ${CAIRO_LIBRARY_DIRS}
    ${SYSTEMD_LIBRARY_DIRS}
    ${XCURSOR_LIBRARY_DIRS}  # Added for Xcursor support
    -lm
)

# List all source files explicitly
set(SOURCES

    src/main.c
    src/wm.c
    src/client.c
    src/display_manager.c
    src/desktop_window.c
    src/audio.c
    src/input.c
    src/ini.c
    src/notifications.c
    src/config.c
    src/wm_interface.c  # Add this line if needed
)

# Create the executable
add_executable(CanopyWM ${SOURCES})

# Link all required libraries
target_link_libraries(CanopyWM
    ${GLIB2_LIBRARIES}
    ${LIBNOTIFY_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${X11_LIBRARIES}
    ${XRANDR_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${SYSTEMD_LIBRARIES}
    ${XCURSOR_LIBRARIES}  # Added for Xcursor support
    pthread
    dl
    m
)
