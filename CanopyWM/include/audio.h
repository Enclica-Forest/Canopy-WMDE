#ifndef CANOPY_AUDIO_H
#define CANOPY_AUDIO_H

#include <alsa/asoundlib.h>
#include <stdbool.h>

typedef struct {
    snd_mixer_t *mixer;
    snd_mixer_elem_t *volume_elem;
    bool muted;
} AudioManager;

void audio_manager_init(void);
void audio_manager_cleanup(void);
void audio_set_volume(int volume);
void audio_toggle_mute(void);
int audio_get_volume(void);
bool audio_is_muted(void);

#endif