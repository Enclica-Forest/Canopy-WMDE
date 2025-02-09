// src/widgets/network.c
#include "widgets/network.h"
#include <libnm/NetworkManager.h>

static NMClient *client = NULL;
static GtkWidget *label = NULL;

/* Callback to open the network preferences */
static void on_network_preferences_clicked(GtkButton *button, gpointer user_data) {
    (void) button;
    (void) user_data;
    /* Launch the network preferences tool.
       Replace "nm-connection-editor" with your preferred command if needed.
       Note: Depending on your environment you might need to adjust the command. */
    g_spawn_command_line_async("nm-connection-editor", NULL);
}

void network_update(GtkWidget *widget) {
    if (!client) return;

    const GPtrArray *devices = nm_client_get_devices(client);
    gboolean connected = FALSE;
    gchar *status = g_strdup("Disconnected");

    for (guint i = 0; i < devices->len; i++) {
        NMDevice *device = NM_DEVICE(devices->pdata[i]);
        NMDeviceState state = nm_device_get_state(device);

        if (NM_IS_DEVICE_WIFI(device)) {
            NMAccessPoint *ap = nm_device_wifi_get_active_access_point(NM_DEVICE_WIFI(device));
            if (ap && state == NM_DEVICE_STATE_ACTIVATED) {
                g_free(status);
                status = g_strdup("WiFi");
                connected = TRUE;
                break;
            }
        }
        else if (NM_IS_DEVICE_ETHERNET(device) && state == NM_DEVICE_STATE_ACTIVATED) {
            g_free(status);
            status = g_strdup("Ethernet");
            connected = TRUE;
            break;
        }
    }

    gtk_label_set_text(GTK_LABEL(label), status);
    g_free(status);
}

GtkWidget *network_new(void) {
    /* Create a horizontal box to contain an icon, status label, and a preferences button */
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    /* Network icon */
    GtkWidget *icon = gtk_image_new_from_icon_name("network-wireless-symbolic", GTK_ICON_SIZE_MENU);
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);

    /* Status label */
    label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

    /* Network Preferences button */
    GtkWidget *pref_button = gtk_button_new_from_icon_name("preferences-system-network", GTK_ICON_SIZE_MENU);
    gtk_widget_set_tooltip_text(pref_button, "Network Preferences");
    g_signal_connect(pref_button, "clicked", G_CALLBACK(on_network_preferences_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), pref_button, FALSE, FALSE, 0);

    /* Create the NetworkManager client */
    GError *error = NULL;
    client = nm_client_new(NULL, &error);
    if (error) {
        g_warning("NetworkManager error: %s", error->message);
        g_error_free(error);
    }

    return box;
}
