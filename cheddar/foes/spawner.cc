#include "spawner.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "object.h"
#include "save_data.h"
#include "scene.h"

FoeSpawner* spawners[20];

FoeSpawner::FoeSpawner(float x, float y, int spawner_id, int animation,
                       const std::vector<std::vector<std::string>>& waves)
    : x(x), y(y), spawner_id(spawner_id), waves(waves) {
  char key[256];
  snprintf(key, sizeof(key), "spawner_%d", this->spawner_id);
  if (thoom::save.geti(key) == 1) {
    this->empty = true;
    this->wave = this->waves.size();
  }

  spawners[spawner_id] = this;
}

FoeSpawner::~FoeSpawner() { spawners[this->spawner_id] = nullptr; }

void FoeSpawner::step() {
  if (!this->to_spawn.empty() &&
      thoom::Scene::instance->ticks - this->spawned_ticks > 500) {
    this->spawned_ticks = thoom::Scene::instance->ticks;

    std::string obj = this->to_spawn.back();
    this->to_spawn.pop_back();

    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d", (int)this->x, (int)this->y,
             this->spawner_id);
    thoom::Scene::instance->create_object(obj, buff);
  }
}

void FoeSpawner::trigger() {
  this->foerefs = 0;

  if (this->wave >= this->waves.size()) {
    this->empty = true;
    char key[256];
    snprintf(key, sizeof(key), "spawner_%d", this->spawner_id);
    thoom::save.puti(key, 1);
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

thoom::Object* FoeSpawner::create_object(thoom::Scene* scene,
                                         const std::string& options) {
  const char* arr = options.c_str();

  int x = 0, y = 0, spawner_id = 0, animation = 0;
  if (4 != sscanf(arr, "%d,%d,%d,%d", &x, &y, &spawner_id, &animation)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  // skip until null byte or space
  while (*arr != '\0' && *arr != ' ') arr++;

  if (*arr == ' ') arr++;  // don't want the last space

  std::vector<std::vector<std::string>> waves;

  // parse waves of enemies
  while (*arr != '\0') {
    char buff[256];
    int i = 0;
    for (; i < sizeof(buff) - 1 && *arr != ',' && *arr != ';' && *arr != '\0';
         i++)
      buff[i] = *arr++;
    buff[i] = '\0';

    if (i == 0) break;

    if (waves.empty()) waves.push_back(std::vector<std::string>());

    waves.back().push_back(std::string(buff));

    if (*arr == ';') waves.push_back(std::vector<std::string>());

    if (*arr != '\0') arr++;
  }

  return new FoeSpawner(x, y, spawner_id, animation, waves);
}
