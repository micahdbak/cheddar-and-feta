#pragma once

#include "../hurtbox.h"
#include "object.h"
#include "sprite.h"

#define BELL_OBJ "bell"

class Bell : public thoom::Object, public HurtSource {
 public:
  Bell(int x, int y, int spawner_id);
  ~Bell();

  void step() override;

  float hurtsource_x() override { return this->x; }
  float hurtsource_y() override { return this->y; }

  void attack(int damage) override;

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
  float x, y;
  int spawner_id;
  bool triggered = false;
  Uint64 timer = 0;
};

class BellFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) {
    int x = 0, y = 0, spawner_id = 0;
    if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id))
      FATAL_ERROR

    return new Bell(x, y, spawner_id);
  }
};
