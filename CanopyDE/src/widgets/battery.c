#include "widgets/battery.h"

typedef struct {
    GtkWidget *button;
    GtkWidget *icon;
    GtkWidget *label;
    UpClient *client;
    UpDevice *device;
    gulong signal_id;
} BatteryWidget;

static void update_battery_status(BatteryWidget *bat) {
    if (!bat->device) {
        gtk_widget_hide(bat->button);
        return;
    }
    
    gdouble percentage;
    guint state;
    gboolean present;
    
    g_object_get(bat->device,
                "percentage", &percentage,
                "state", &state,
                "is-present", &present,
                NULL);
    
    if (!present) {
        gtk_widget_hide(bat->button);
        return;
    }
    
    const char *icon_name;
    if (state == UP_DEVICE_STATE_CHARGING) {
        if (percentage < 10)
            icon_name = "battery-caution-charging-symbolic";
        else if (percentage < 30)
            icon_name = "battery-low-charging-symbolic";
        else if (percentage < 60)
            icon_name = "battery-good-charging-symbolic";
        else
            icon_name = "battery-full-charging-symbolic";
    } else {
        if (percentage < 10)
            icon_name = "battery-caution-symbolic";
        else if (percentage < 30)
            icon_name = "battery-low-symbolic";
        else if (percentage < 60)
            icon_name = "battery-good-symbolic";
        else
            icon_name = "battery-full-symbolic";
    }
    
    gtk_image_set_from_icon_name(GTK_IMAGE(bat->icon), icon_name, GTK_ICON_SIZE_MENU);
    
    char *text = g_strdup_printf("%.0f%%", percentage);
    gtk_label_set_text(GTK_LABEL(bat->label), text);
    g_free(text);
    
    gtk_widget_show(bat->button);
}

static void on_device_changed(UpDevice *device, GParamSpec *pspec, BatteryWidget *bat) {
    update_battery_status(bat);
}

GtkWidget *battery_new(void) {
    BatteryWidget *bat = g_new0(BatteryWidget, 1);
    
    bat->client = up_client_new();
    bat->button = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    bat->icon = gtk_image_new();
    bat->label = gtk_label_new("");
    
    gtk_box_pack_start(GTK_BOX(bat->button), bat->icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bat->button), bat->label, FALSE, FALSE, 0);
    
    GPtrArray *devices = up_client_get_devices2(bat->client);
    for (guint i = 0; i < devices->len; i++) {
        UpDevice *device = g_ptr_array_index(devices, i);
        guint kind;
        g_object_get(device, "kind", &kind, NULL);
        if (kind == UP_DEVICE_KIND_BATTERY) {
            bat->device = g_object_ref(device);
            bat->signal_id = g_signal_connect(device, "notify",
                                            G_CALLBACK(on_device_changed), bat);
            break;
        }
    }
    g_ptr_array_unref(devices);
    
    update_battery_status(bat);
    
    g_object_set_data_full(G_OBJECT(bat->button), "battery-data", bat,
                          (GDestroyNotify)g_free);
    
    return bat->button;
}

void battery_update(GtkWidget *battery) {
    BatteryWidget *bat = g_object_get_data(G_OBJECT(battery), "battery-data");
    if (bat) {
        update_battery_status(bat);
    }
}