cmake_minimum_required(VERSION 3.10)
project(CanopyDE VERSION 1.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CANOPY_DATADIR "${CMAKE_INSTALL_PREFIX}/share/canopy" CACHE PATH "Data directory for Canopy")
add_definitions(-DCANOPY_DATADIR="${CANOPY_DATADIR}")

# Find pkg-config and required packages
find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(NETWORKMANAGER REQUIRED libnm)
pkg_check_modules(UPOWER REQUIRED upower-glib)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(X11 REQUIRED x11)

# Source files
set(SOURCES
    src/main.c
    src/desktop.c
    src/panel.c
    src/settings.c
    src/taskbar.c
    src/systray.c
    src/widgets/battery.c
    src/widgets/clock.c
    src/widgets/menu_button.c
    src/widgets/network.c
    src/widgets/volume.c
)

add_executable(canopy-de ${SOURCES})

target_include_directories(canopy-de PRIVATE
    include
    ${GTK3_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
    ${NETWORKMANAGER_INCLUDE_DIRS}
    ${UPOWER_INCLUDE_DIRS}
)

target_link_libraries(canopy-de
    ${GTK3_LIBRARIES}
    ${GIO_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
    ${NETWORKMANAGER_LIBRARIES}
    ${UPOWER_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${X11_LIBRARIES}
    -lm
)

install(TARGETS canopy-de DESTINATION bin)
