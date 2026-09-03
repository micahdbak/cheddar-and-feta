#include "audio_playback.h"

#include <iostream>
#include <unordered_map>

#include "game.h"

#define NUM_VOICES 32

#define IDLE_THRESHOLD 4096      // 4KiB
#define LOOP_THRESHOLD 4096 * 8  // 32KiB

// essentially, audio < LISTEN_NEAR px of the listener is at max volume
// audio > LISTEN_NEAR & < LISTEN_FAR are quieter, audio > LISTEN_FAR are silent
#define LISTEN_NEAR 160.0f
#define LISTEN_FAR 320.0f

struct audio_source {
  SDL_AudioSpec spec;
  Uint8* buf;
  Uint32 len;
};

static std::unordered_map<std::string, struct audio_source> audio_sources;
static SDL_AudioDeviceID playback_device = 0;
static SDL_AudioSpec playback_spec;
static SDL_AudioStream* voices[NUM_VOICES] = {0};
static SDL_AudioSpec voice_spec[NUM_VOICES];
static struct audio_source* ambience = NULL;
static float listener_x = 0.0f, listener_y = 0.0f;
static int current_music_volume = 100;

void init_playback() {
  playback_device =
      SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

  if (playback_device == 0) {
    std::cerr << "init_playback: SDL_OpenAudioDevice: " << SDL_GetError()
              << std::endl;
    std::exit(1);
  }

  if (!SDL_GetAudioDeviceFormat(playback_device, &playback_spec, NULL)) {
    std::cerr << "init_playback: SDL_GetAudioDeviceFormat: " << SDL_GetError()
              << std::endl;
    std::exit(1);
  }

  for (int i = 0; i < NUM_VOICES; i++) {
    voices[i] = SDL_CreateAudioStream(&playback_spec, &playback_spec);
    SDL_BindAudioStream(playback_device, voices[i]);
  }

  load_audio("sfx/ant_attack.wav");
  load_audio("sfx/ant_die.wav");
  load_audio("sfx/ant_fly.wav");
  load_audio("sfx/ant_hurt.wav");
  load_audio("sfx/ant_walk1.wav");
  load_audio("sfx/ant_walk2.wav");
  load_audio("sfx/ant_walk3.wav");
  load_audio("sfx/bell.wav");
  load_audio("sfx/blip.wav");
  load_audio("sfx/cannon.wav");
  load_audio("sfx/cheese.wav");
  load_audio("sfx/door.wav");
  load_audio("sfx/fire.wav");
  load_audio("sfx/fire_short1.wav");
  load_audio("sfx/fire_short2.wav");
  load_audio("sfx/full.wav");
  load_audio("sfx/hurt.wav");
  load_audio("sfx/kick.wav");
  load_audio("sfx/ladder.wav");
  load_audio("sfx/pickup.wav");
  load_audio("sfx/settle.wav");
  load_audio("sfx/spitter_die.wav");
  load_audio("sfx/spitter_hurt.wav");
  load_audio("sfx/spitter_walk1.wav");
  load_audio("sfx/spitter_walk2.wav");
  load_audio("sfx/spitter_walk3.wav");
  load_audio("sfx/spitter_walk4.wav");
  load_audio("sfx/step.wav");
  load_audio("sfx/tank_die.wav");
  load_audio("sfx/tank_hurt.wav");
  load_audio("sfx/tank_roll1.wav");
  load_audio("sfx/tank_roll2.wav");
  load_audio("sfx/throw.wav");
}

void free_playback() {
  SDL_CloseAudioDevice(playback_device);

  for (int i = 0; i < NUM_VOICES; i++) {
    SDL_DestroyAudioStream(voices[i]);
    voices[i] = nullptr;
  }

  for (auto it = audio_sources.begin(); it != audio_sources.end(); it++) {
    struct audio_source& src = (*it).second;
    SDL_free(src.buf);
  }

  audio_sources.clear();
}

void load_audio(const std::string& wav_path) {
  if (wav_path.empty() || audio_sources.find(wav_path) != audio_sources.end()) {
    return;
  }

  struct audio_source src;

  if (!SDL_LoadWAV(wav_path.c_str(), &src.spec, &src.buf, &src.len)) {
    std::cerr << "load_audio_stream: SDL_LoadWAV: " << SDL_GetError()
              << std::endl;
    std::exit(1);
  }

  audio_sources[wav_path] = src;
}

void set_listener(float x, float y) {
  listener_x = x;
  listener_y = y;
}

void play_audio(const std::string& wav_path, float gain, float x, float y,
                bool from_feta) {
  auto it = audio_sources.find(wav_path);

  if (it == audio_sources.end()) {
    return;
  }

  if (!from_feta) {
    game->audio.push_back(Game::AudioMsg{wav_path, gain, x, y});
  }

  float dist = distance_between_points(listener_x, listener_y, x, y);

  // too far
  if (dist > LISTEN_FAR) {
    return;
  }

  if (dist > LISTEN_NEAR) {
    float mult = (dist - LISTEN_NEAR) / (LISTEN_FAR - LISTEN_NEAR);
    mult = 1.0f - mult;
    gain *= mult;
  }

  gain *= (float)game->volume / 200.0f;

  struct audio_source& src = (*it).second;

  // voices[0] & voices[1] is reserved for ambient audio
  for (int i = 1; i < NUM_VOICES; i++) {
    // find an idle audio stream
    if (SDL_GetAudioStreamQueued(voices[i]) > IDLE_THRESHOLD) {
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

void loop_audio(const std::string& wav_path) {
  SDL_ClearAudioStream(voices[0]);

  if (wav_path.empty()) {
    ambience = NULL;
    SDL_ClearAudioStream(voices[0]);
    return;
  }

  auto it = audio_sources.find(wav_path);

  if (it == audio_sources.end()) {
    return;
  }

  ambience = &(*it).second;
  float music_gain = (float)game->music_volume / 200.0f;

  if (!SDL_SetAudioStreamFormat(voices[0], &ambience->spec, NULL) ||
      !SDL_SetAudioStreamGain(voices[0], music_gain) ||
      !SDL_PutAudioStreamData(voices[0], ambience->buf, ambience->len)) {
    std::cerr << "loop_audio: " << SDL_GetError() << std::endl;
    std::exit(1);
  }
}

void ambience_step() {
  float music_gain = (float)game->music_volume / 200.0f;

  if (current_music_volume != game->music_volume &&
      !SDL_SetAudioStreamGain(voices[0], music_gain)) {
    current_music_volume = game->music_volume;
    std::cerr << "loop_audio: " << SDL_GetError() << std::endl;
    std::exit(1);
  }

  // wait until < approx 32kb of audio remains
  if (ambience == NULL ||
      SDL_GetAudioStreamQueued(voices[0]) >= LOOP_THRESHOLD) {
    return;
  }

  if (!SDL_PutAudioStreamData(voices[0], ambience->buf, ambience->len)) {
    std::cerr << "ambience_step: " << SDL_GetError() << std::endl;
    std::exit(1);
  }
}