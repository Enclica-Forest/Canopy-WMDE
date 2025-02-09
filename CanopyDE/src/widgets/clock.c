// src/widgets/clock.c
#include "widgets/clock.h"
#include <time.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>

/* ------------------------------
   CLOCK: Updates every second
   ------------------------------ */
static gboolean update_clock(gpointer data) {
    GtkLabel *label = GTK_LABEL(data);
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm);
    gtk_label_set_text(label, time_str);
    return G_SOURCE_CONTINUE;
}

GtkWidget *clock_new(void) {
    GtkWidget *label = gtk_label_new(NULL);
    update_clock(label);
    g_timeout_add_seconds(1, update_clock, label);
    return label;
}

void clock_update(GtkWidget *clock) {
    update_clock(clock);
}

/* ------------------------------
   CALENDAR & EVENT SYSTEM
   ------------------------------ */
typedef struct {
    time_t event_time;
    char *description;
} CalendarEvent;

static GList *event_list = NULL;

/* Play a sound when an event is triggered.
   Adjust the command to your environment. */
static void play_event_sound(void) {
    g_spawn_command_line_async(
        "paplay /usr/share/sounds/freedesktop/stereo/complete.oga", NULL);
}

/* Check the event list every minute.
   If an event's time is <= current time, play a sound and remove the event. */
static gboolean check_events(gpointer data) {
    (void)data;
    time_t now = time(NULL);
    GList *iter = event_list;
    while (iter) {
        CalendarEvent *event = iter->data;
        if (event->event_time <= now) {
            play_event_sound();
            g_print("Event triggered: %s\n", event->description);
            /* Remove event from list and free its memory */
            GList *to_remove = iter;
            iter = iter->next;
            event_list = g_list_remove_link(event_list, to_remove);
            g_free(event->description);
            g_free(event);
            g_list_free_1(to_remove);
        } else {
            iter = iter->next;
        }
    }
    return G_SOURCE_CONTINUE;
}

/* Add a new event.
   For simplicity, the event time is specified in local time. */
static void add_calendar_event(int year, int month, int day, int hour, int min,
                               const char *description) {
    struct tm tm_time = {0};
    tm_time.tm_year = year - 1900;   // struct tm year is years since 1900
    tm_time.tm_mon  = month;         // GtkCalendar returns month 0-11
    tm_time.tm_mday = day;
    tm_time.tm_hour = hour;
    tm_time.tm_min  = min;
    tm_time.tm_sec  = 0;
    time_t event_time = mktime(&tm_time);
    
    CalendarEvent *event = g_new(CalendarEvent, 1);
    event->event_time = event_time;
    event->description = g_strdup(description);
    event_list = g_list_append(event_list, event);
    g_print("Added event: \"%s\" at %s", description, ctime(&event_time));
}

/* Callback for the "Add Event" button in the calendar widget.
   Expects an array of two GtkWidget pointers: the calendar and the entry. */
static void on_add_event_clicked(GtkButton *button, gpointer user_data) {
    (void) button;
    GtkWidget **widgets = user_data;
    GtkCalendar *calendar = GTK_CALENDAR(widgets[0]);
    GtkEntry *entry = GTK_ENTRY(widgets[1]);
    
    guint year, month, day;
    gtk_calendar_get_date(calendar, &year, &month, &day);
    
    const char *desc = gtk_entry_get_text(entry);
    if (desc && desc[0] != '\0') {
        /* For this example, assume the event occurs at 09:00 on the selected day. */
        add_calendar_event(year, month, day, 9, 0, desc);
        gtk_entry_set_text(entry, ""); // clear the entry after adding
    }
}

/* Create a calendar widget that lets the user add events.
   Returns a container with a GtkCalendar, a text entry, and an "Add Event" button. */
GtkWidget *calendar_new(void) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    
    GtkWidget *calendar = gtk_calendar_new();
    gtk_box_pack_start(GTK_BOX(vbox), calendar, FALSE, FALSE, 0);
    
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Event description");
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
    
    GtkWidget *add_button = gtk_button_new_with_label("Add Event");
    gtk_box_pack_start(GTK_BOX(vbox), add_button, FALSE, FALSE, 0);
    
    /* Allocate an array to hold calendar and entry pointers as user data */
    GtkWidget **widgets = g_new(GtkWidget*, 2);
    widgets[0] = calendar;
    widgets[1] = entry;
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_event_clicked), widgets);
    
    return vbox;
}

/* Call this during application initialization to start the event checking timer */
void calendar_init(void) {
    g_timeout_add_seconds(60, check_events, NULL);
}
