#include "spawner.h"

#include <iostream>

#include "audio_playback.h"
#include "controller.h"
#include "game.h"
#include "save_data.h"

FoeSpawner* spawners[20];

FoeSpawner::FoeSpawner(float x, float y, int spawner_id, int animation,
                       const std::vector<std::vector<std::string>>& waves)
    : x(x), y(y), spawner_id(spawner_id), waves(waves) {
  char key[256];
  snprintf(key, sizeof(key), "spawner_%d", this->spawner_id);
  if (save.geti(key) == 1) {
    this->empty = true;
    this->wave = this->waves.size();
  }

  spawners[spawner_id] = this;
}

FoeSpawner::~FoeSpawner() { spawners[this->spawner_id] = nullptr; }

void FoeSpawner::step() {
  if (!this->to_spawn.empty() && game->ticks - this->spawned_ticks > 500) {
    this->spawned_ticks = game->ticks;

    std::string obj = this->to_spawn.back();
    this->to_spawn.pop_back();

    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d", (int)this->x, (int)this->y,
             this->spawner_id);
    game->push_object(obj, buff);
  }
}

void FoeSpawner::trigger() {
  this->foerefs = 0;

  if (this->wave >= this->waves.size()) {
    this->empty = true;
    char key[256];
    snprintf(key, sizeof(key), "spawner_%d", this->spawner_id);
    save.puti(key, 1);
    return;
  }

  this->to_spawn = this->waves[this->wave];
  this->foerefs += this->to_spawn.size();
  this->wave++;
}

void FoeSpawner::log_death() {
  this->foerefs--;

  if (this->foerefs <= 0) {
    this->trigger();
  }
}
