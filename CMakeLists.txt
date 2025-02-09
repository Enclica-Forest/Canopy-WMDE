cmake_minimum_required(VERSION 3.10)
project(Canopy VERSION 1.0)

# Find common dependencies once at the top level
find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(XRANDR REQUIRED xrandr)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(NOTIFY REQUIRED libnotify)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GIO REQUIRED gio-2.0 gio-unix-2.0)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)


# Add subdirectories
add_subdirectory(CanopyWM)
add_subdirectory(CanopyDE)