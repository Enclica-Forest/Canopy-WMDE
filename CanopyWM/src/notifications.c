
// src/notifications.c
#include "notifications.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global notification manager instance
NotificationManager notification_manager;

void notification_manager_init(void) {
    if (!notify_init("CanopyWM")) {
        fprintf(stderr, "Failed to initialize libnotify\n");
        return;
    }
    
    notification_manager.count = 0;
    notification_manager.initialized = true;
}

void notification_manager_cleanup(void) {
    for (int i = 0; i < notification_manager.count; i++) {
        if (notification_manager.entries[i].notification) {
            notify_notification_close(notification_manager.entries[i].notification, NULL);
        }
        free(notification_manager.entries[i].summary);
        free(notification_manager.entries[i].body);
    }
    
    notification_manager.count = 0;
    notification_manager.initialized = false;
    notify_uninit();
}

void notification_show(const char *summary, const char *body, int timeout) {
    if (!notification_manager.initialized || 
        notification_manager.count >= MAX_NOTIFICATIONS) {
        return;
    }
    
    NotificationEntry *entry = &notification_manager.entries[notification_manager.count++];
    
    entry->notification = notify_notification_new(summary, body, NULL);
    notify_notification_set_timeout(entry->notification, timeout);
    
    entry->summary = strdup(summary);
    entry->body = strdup(body);
    entry->timestamp = time(NULL);
    entry->timeout = timeout;
    
    notify_notification_show(entry->notification, NULL);
}

void notification_clear_expired(void) {
    time_t current_time = time(NULL);
    int i = 0;
    
    while (i < notification_manager.count) {
        NotificationEntry *entry = &notification_manager.entries[i];
        
        if (current_time - entry->timestamp > entry->timeout / 1000) {
            // Close and free notification
            notify_notification_close(entry->notification, NULL);
            free(entry->summary);
            free(entry->body);
            
            // Move remaining notifications up
            memmove(&notification_manager.entries[i],
                   &notification_manager.entries[i + 1],
                   (notification_manager.count - i - 1) * sizeof(NotificationEntry));
            
            notification_manager.count--;
        } else {
            i++;
        }
    }
}

void notification_clear_all(void) {
    for (int i = 0; i < notification_manager.count; i++) {
        if (notification_manager.entries[i].notification) {
            notify_notification_close(notification_manager.entries[i].notification, NULL);
        }
        free(notification_manager.entries[i].summary);
        free(notification_manager.entries[i].body);
    }
    
    notification_manager.count = 0;
}