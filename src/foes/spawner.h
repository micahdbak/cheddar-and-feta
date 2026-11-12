#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#include "object/entity.h"

#define SPAWNER_OBJ "spawner"

class FoeSpawner : public thoom::Entity {
 public:
  FoeSpawner(const thoom::Entity::Context& context, float x, float y,
             int spawner_id, int animation,
             const std::vector<std::vector<std::string>>& waves);
  ~FoeSpawner();

  void step() override;

  void trigger();

  void log_death();

  bool empty = false;
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  float x, y;
  int spawner_id;
  std::vector<std::vector<std::string>> waves;
  std::vector<std::string> to_spawn;
  Uint64 spawned_ticks = 0;

  int wave = 0;
  int foerefs = 0;
};

extern FoeSpawner* spawners[];
