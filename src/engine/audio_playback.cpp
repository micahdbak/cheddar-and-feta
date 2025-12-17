#include "audio_playback.h"

#include <unordered_map>
#include <iostream>

#define NUM_VOICES 32

struct audio_source {
    SDL_AudioSpec spec;
    Uint8 *buf;
    Uint32 len;
};

static std::unordered_map<std::string, struct audio_source> audio_sources;
static SDL_AudioDeviceID playback_device = 0;
static SDL_AudioSpec playback_spec;
static SDL_AudioStream *voices[NUM_VOICES] = {0};
static SDL_AudioSpec voice_spec[NUM_VOICES];
static struct audio_source *ambience = NULL;

void init_playback() {
    playback_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

    if (playback_device == 0) {
        std::cerr << "init_playback: SDL_OpenAudioDevice: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    if (!SDL_GetAudioDeviceFormat(playback_device, &playback_spec, NULL)) {
        std::cerr << "init_playback: SDL_GetAudioDeviceFormat: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    for (int i = 0; i < NUM_VOICES; i++) {
        voices[i] = SDL_CreateAudioStream(&playback_spec, &playback_spec);
        SDL_BindAudioStream(playback_device, voices[i]);
    }
}

void free_playback() {
    SDL_CloseAudioDevice(playback_device);

    for (int i = 0; i < NUM_VOICES; i++) {
        SDL_DestroyAudioStream(voices[i]);
        voices[i] = nullptr;
    }

    for (auto it = audio_sources.begin(); it != audio_sources.end(); it++) {
        struct audio_source &src = (*it).second;
        SDL_free(src.buf);
    }

    audio_sources.clear();
}

void load_audio(const std::string &wav_path) {
    if (wav_path.empty() || audio_sources.find(wav_path) != audio_sources.end()) {
        return;
    }

    struct audio_source src;

    if (!SDL_LoadWAV(wav_path.c_str(), &src.spec, &src.buf, &src.len)) {
        std::cerr << "load_audio_stream: SDL_LoadWAV: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    audio_sources[wav_path] = src;
}

void play_audio(const std::string &wav_path, float gain) {
    auto it = audio_sources.find(wav_path);

    if (it == audio_sources.end()) {
        return;
    }

    struct audio_source &src = (*it).second;

    // voices[0] is reserved for ambient audio
    for (int i = 1; i < NUM_VOICES; i++) {
        // find an idle audio stream
        if (SDL_GetAudioStreamQueued(voices[i]) > 0) {
            continue;
        }

        SDL_ClearAudioStream(voices[i]);

        // set stream format + start playing
        if (!SDL_SetAudioStreamFormat(voices[i], &src.spec, NULL) ||
            !SDL_SetAudioStreamGain(voices[i], gain) ||
            !SDL_PutAudioStreamData(voices[i], src.buf, src.len)) {
            std::cerr << "play_audio: " << SDL_GetError() << std::endl;
            std::exit(1);
        }

        break;
    }
}

void loop_audio(const std::string &wav_path) {
    SDL_ClearAudioStream(voices[0]);

    if (wav_path.empty()) {
        ambience = NULL;
        return;
    }

    auto it = audio_sources.find(wav_path);

    if (it == audio_sources.end()) {
        return;
    }

    ambience = &(*it).second;

    if (!SDL_SetAudioStreamFormat(voices[0], &ambience->spec, NULL) ||
        !SDL_PutAudioStreamData(voices[0], ambience->buf, ambience->len)) {
        std::cerr << "loop_audio: " << SDL_GetError() << std::endl;
        std::exit(1);
    }
}

void ambience_step() {
    // wait until < approx 32kb of audio remains
    if (ambience == NULL || SDL_GetAudioStreamQueued(voices[0]) >= 32000) {
        return;
    }

    if (!SDL_PutAudioStreamData(voices[0], ambience->buf, ambience->len)) {
        std::cerr << "ambience_step: " << SDL_GetError() << std::endl;
        std::exit(1);
    }
}