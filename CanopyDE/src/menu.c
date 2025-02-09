// src/menu.c
#include <gtk/gtk.h>
#include "menu.h"
#include "launcher.h"

typedef struct {
    GtkWidget *menu;
    GtkWidget *applications;
    GtkWidget *places;
    GtkWidget *system;
} MainMenu;

static MainMenu main_menu;

static void on_applications_activate(GtkMenuItem *item, gpointer data) {
    launcher_show();
}

static void on_logout_activate(GtkMenuItem *item, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                             GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_QUESTION,
                                             GTK_BUTTONS_YES_NO,
                                             "Do you want to log out?");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
        gtk_main_quit();
    }
    
    gtk_widget_destroy(dialog);
}

void menu_init(void) {
    main_menu.menu = gtk_menu_new();
    
    // Applications menu
    main_menu.applications = gtk_menu_item_new_with_label("Applications");
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu.menu), main_menu.applications);
    g_signal_connect(main_menu.applications, "activate",
                    G_CALLBACK(on_applications_activate), NULL);
    
    // Places menu
    main_menu.places = gtk_menu_item_new_with_label("Places");
    GtkWidget *places_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_menu.places), places_menu);
    
    GtkWidget *home = gtk_menu_item_new_with_label("Home");
    gtk_menu_shell_append(GTK_MENU_SHELL(places_menu), home);
    
    GtkWidget *documents = gtk_menu_item_new_with_label("Documents");
    gtk_menu_shell_append(GTK_MENU_SHELL(places_menu), documents);
    
    GtkWidget *pictures = gtk_menu_item_new_with_label("Pictures");
    gtk_menu_shell_append(GTK_MENU_SHELL(places_menu), pictures);
    
    GtkWidget *downloads = gtk_menu_item_new_with_label("Downloads");
    gtk_menu_shell_append(GTK_MENU_SHELL(places_menu), downloads);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu.menu), main_menu.places);
    
    // System menu
    main_menu.system = gtk_menu_item_new_with_label("System");
    GtkWidget *system_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_menu.system), system_menu);
    
    GtkWidget *settings = gtk_menu_item_new_with_label("Settings");
    gtk_menu_shell_append(GTK_MENU_SHELL(system_menu), settings);
    
    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(system_menu), separator);
    
    GtkWidget *logout = gtk_menu_item_new_with_label("Log Out");
    gtk_menu_shell_append(GTK_MENU_SHELL(system_menu), logout);
    g_signal_connect(logout, "activate", G_CALLBACK(on_logout_activate), NULL);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu.menu), main_menu.system);
    
    gtk_widget_show_all(main_menu.menu);
}

GtkWidget *menu_get(void) {
    return main_menu.menu;
}