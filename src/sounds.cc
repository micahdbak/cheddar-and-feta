#include "sounds.h"

#include "audio/audio.h"

void load_sounds() {
  thoom::Audio::instance->load("sfx/ant_attack.wav");
  thoom::Audio::instance->load("sfx/ant_die.wav");
  thoom::Audio::instance->load("sfx/ant_fly.wav");
  thoom::Audio::instance->load("sfx/ant_hurt.wav");
  thoom::Audio::instance->load("sfx/ant_walk1.wav");
  thoom::Audio::instance->load("sfx/ant_walk2.wav");
  thoom::Audio::instance->load("sfx/ant_walk3.wav");
  thoom::Audio::instance->load("sfx/bell.wav");
  thoom::Audio::instance->load("sfx/blip.wav");
  thoom::Audio::instance->load("sfx/cannon.wav");
  thoom::Audio::instance->load("sfx/cheese.wav");
  thoom::Audio::instance->load("sfx/door.wav");
  thoom::Audio::instance->load("sfx/fire.wav");
  thoom::Audio::instance->load("sfx/fire_short1.wav");
  thoom::Audio::instance->load("sfx/fire_short2.wav");
  thoom::Audio::instance->load("sfx/full.wav");
  thoom::Audio::instance->load("sfx/hurt.wav");
  thoom::Audio::instance->load("sfx/kick.wav");
  thoom::Audio::instance->load("sfx/ladder.wav");
  thoom::Audio::instance->load("sfx/pickup.wav");
  thoom::Audio::instance->load("sfx/settle.wav");
  thoom::Audio::instance->load("sfx/spitter_die.wav");
  thoom::Audio::instance->load("sfx/spitter_hurt.wav");
  thoom::Audio::instance->load("sfx/spitter_walk1.wav");
  thoom::Audio::instance->load("sfx/spitter_walk2.wav");
  thoom::Audio::instance->load("sfx/spitter_walk3.wav");
  thoom::Audio::instance->load("sfx/spitter_walk4.wav");
  thoom::Audio::instance->load("sfx/step.wav");
  thoom::Audio::instance->load("sfx/tank_die.wav");
  thoom::Audio::instance->load("sfx/tank_hurt.wav");
  thoom::Audio::instance->load("sfx/tank_roll1.wav");
  thoom::Audio::instance->load("sfx/tank_roll2.wav");
  thoom::Audio::instance->load("sfx/throw.wav");
}
