                    // src/main.c
                    #include <gtk/gtk.h>
                    #include "panel.h"
                    #include "desktop.h"
                    #include "settings.h"
                    #include "systray.h"

                    static void cleanup(void) {
                        panel_cleanup();
                        systray_cleanup();
                        settings_save();
                    }

                    int main(int argc, char *argv[]) {
                        gtk_init(&argc, &argv);
                        
                        // Initialize components
                        settings_init();
                        desktop_init();

                        panel_init();
                        systray_init();

                        // Set default wallpaper if available
                        char *wallpaper = g_build_filename(CANOPY_DATADIR, "backgrounds",
                                                        "default.jpg", NULL);
                        if (g_file_test(wallpaper, G_FILE_TEST_EXISTS)) {
                            desktop_set_wallpaper(wallpaper);
                        }
                        g_free(wallpaper);
                        
                        // Set up cleanup
                        atexit(cleanup);
                        
                        // Run main loop
                        gtk_main();
                        
                        return 0;
                    }