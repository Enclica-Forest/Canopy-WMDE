#include "audio.h"
#include <stdlib.h>

static AudioManager audio_manager;

void audio_manager_init(void) {
    snd_mixer_open(&audio_manager.mixer, 0);
    snd_mixer_attach(audio_manager.mixer, "default");
    snd_mixer_selem_register(audio_manager.mixer, NULL, NULL);
    snd_mixer_load(audio_manager.mixer);
    
    snd_mixer_selem_id_t *sid;
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, "Master");
    
    audio_manager.volume_elem = snd_mixer_find_selem(audio_manager.mixer, sid);
    audio_manager.muted = false;
}
void audio_manager_cleanup(void) {
    if (audio_manager.mixer) {
        snd_mixer_close(audio_manager.mixer);
        audio_manager.mixer = NULL;
    }
    audio_manager.volume_elem = NULL;
}
void audio_set_volume(int volume) {
    long min, max;
    snd_mixer_selem_get_playback_volume_range(audio_manager.volume_elem, &min, &max);
    long val = (volume * (max - min) / 100) + min;
    snd_mixer_selem_set_playback_volume_all(audio_manager.volume_elem, val);
}
