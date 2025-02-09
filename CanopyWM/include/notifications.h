// include/notifications.h
#ifndef CANOPY_NOTIFICATIONS_H
#define CANOPY_NOTIFICATIONS_H

#include <libnotify/notify.h>
#include <time.h>
#include <stdbool.h>
#include <libnotify/notification.h>

#define MAX_NOTIFICATIONS 32
#define NOTIFICATION_TIMEOUT 5000 // 5 seconds

typedef struct {
    NotifyNotification *notification;
    char *summary;
    char *body;
    time_t timestamp;
    int timeout;
} NotificationEntry;

typedef struct {
    NotificationEntry entries[MAX_NOTIFICATIONS];
    int count;
    bool initialized;
} NotificationManager;

// Function declarations
void notification_manager_init(void);
void notification_manager_cleanup(void);
void notification_show(const char *summary, const char *body, int timeout);
void notification_clear_expired(void);
void notification_clear_all(void);

// Global notification manager instance
extern NotificationManager notification_manager;

#endif