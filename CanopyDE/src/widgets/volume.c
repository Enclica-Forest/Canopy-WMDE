// src/widgets/volume.c
#include "widgets/volume.h"
#include <alsa/asoundlib.h>

typedef struct {
    GtkWidget *button;
    GtkWidget *icon;
    GtkWidget *popover;
    GtkWidget *scale;
    snd_mixer_t *mixer;
    snd_mixer_elem_t *elem;
    long min_volume;
    long max_volume;
} VolumeWidget;

/* Forward declaration */
static void update_volume_icon(VolumeWidget *vol);

/* Show the volume popover */
static void show_volume_popover(GtkWidget *button __attribute__((unused)), gpointer user_data) {
    VolumeWidget *vol = user_data;
    gtk_popover_popup(GTK_POPOVER(vol->popover));
}

/* Called when the volume slider is changed */
static void on_volume_changed(GtkRange *range, gpointer user_data) {
    VolumeWidget *vol = user_data;
    /* Get the percentage value from the slider (0 - 100) */
    double percent = gtk_range_get_value(range);
    /* Convert percentage to ALSA scale:
       ALSA volume = min_volume + (percentage/100 * (max_volume - min_volume))
    */
    long new_volume = vol->min_volume + (long)((percent / 100.0) * (vol->max_volume - vol->min_volume));
    snd_mixer_selem_set_playback_volume_all(vol->elem, new_volume);
    update_volume_icon(vol);
}

/* Update the volume icon based on the current volume percentage */
static void update_volume_icon(VolumeWidget *vol) {
    long current;
    snd_mixer_selem_get_playback_volume(vol->elem, SND_MIXER_SCHN_FRONT_LEFT, &current);
    
    /* Calculate the volume percentage relative to the ALSA range */
    long percent = 0;
    if (vol->max_volume != vol->min_volume) {
        percent = ((current - vol->min_volume) * 100) / (vol->max_volume - vol->min_volume);
    }
    
    const char *icon_name;
    if (percent == 0)
        icon_name = "audio-volume-muted-symbolic";
    else if (percent < 33)
        icon_name = "audio-volume-low-symbolic";
    else if (percent < 66)
        icon_name = "audio-volume-medium-symbolic";
    else
        icon_name = "audio-volume-high-symbolic";
    
    gtk_image_set_from_icon_name(GTK_IMAGE(vol->icon), icon_name, GTK_ICON_SIZE_MENU);
}

/* Create a new volume widget */
GtkWidget *volume_new(void) {
    VolumeWidget *vol = g_new0(VolumeWidget, 1);
    
    /* Initialize ALSA mixer */
    if (snd_mixer_open(&vol->mixer, 0) < 0) {
        g_warning("Failed to open mixer");
        return NULL;
    }
    if (snd_mixer_attach(vol->mixer, "default") < 0) {
        g_warning("Failed to attach mixer");
        snd_mixer_close(vol->mixer);
        return NULL;
    }
    snd_mixer_selem_register(vol->mixer, NULL, NULL);
    snd_mixer_load(vol->mixer);
    
    snd_mixer_selem_id_t *sid;
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, "Master");
    vol->elem = snd_mixer_find_selem(vol->mixer, sid);
    if (!vol->elem) {
        g_warning("Failed to find Master mixer element");
        snd_mixer_close(vol->mixer);
        return NULL;
    }
    
    /* Get the ALSA volume range */
    snd_mixer_selem_get_playback_volume_range(vol->elem, &vol->min_volume, &vol->max_volume);
    
    /* Create GTK widgets */
    vol->button = gtk_button_new();
    vol->icon = gtk_image_new_from_icon_name("audio-volume-high-symbolic", GTK_ICON_SIZE_MENU);
    gtk_container_add(GTK_CONTAINER(vol->button), vol->icon);
    
    /* Create popover for volume control */
    vol->popover = gtk_popover_new(vol->button);
    gtk_popover_set_position(GTK_POPOVER(vol->popover), GTK_POS_TOP);
    
    /* Create vertical scale for volume control (0-100 percent) */
    vol->scale = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0, 100, 1);
    gtk_widget_set_size_request(vol->scale, -1, 150);
    gtk_container_add(GTK_CONTAINER(vol->popover), vol->scale);
    
    /* Get current ALSA volume and convert it to percentage */
    long current;
    snd_mixer_selem_get_playback_volume(vol->elem, SND_MIXER_SCHN_FRONT_LEFT, &current);
    double percent = 0;
    if (vol->max_volume != vol->min_volume) {
        percent = ((current - vol->min_volume) * 100.0) / (vol->max_volume - vol->min_volume);
    }
    gtk_range_set_value(GTK_RANGE(vol->scale), percent);
    
    /* Connect signals */
    g_signal_connect(vol->button, "clicked", G_CALLBACK(show_volume_popover), vol);
    g_signal_connect(vol->scale, "value-changed", G_CALLBACK(on_volume_changed), vol);
    
    /* Store the widget data for later retrieval */
    g_object_set_data_full(G_OBJECT(vol->button), "volume-data", vol,
                          (GDestroyNotify)g_free);
    
    update_volume_icon(vol);
    gtk_widget_show_all(vol->scale);
    
    return vol->button;
}

/* External function to update the volume widget (e.g., from a timer) */
void volume_update(GtkWidget *volume) {
    VolumeWidget *vol = g_object_get_data(G_OBJECT(volume), "volume-data");
    if (vol) {
        update_volume_icon(vol);
    }
}
