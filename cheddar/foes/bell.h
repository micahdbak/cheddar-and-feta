#pragma once

#include "../hurtbox.h"
#include "object.h"
#include "scene.h"
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
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
  float x, y;
  int spawner_id;
  bool triggered = false;
  Uint64 timer = 0;
};
