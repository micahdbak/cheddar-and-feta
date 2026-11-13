#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "hitbox.h"
#include "hurtbox.h"
#include "object/entity.h"
#include "sprite/sprite.h"

#define BELL_OBJ "bell"

class Bell : public thoom::Entity, public HitReceiver {
 public:
  Bell(const thoom::Entity::Context& context, int x, int y, int spawner_id);
  ~Bell();

  void step() override;
  void render() override;

  float hurt_x() const override { return this->x; }
  float hurt_y() const override { return this->y; }

  void receive_hit(const Hit& hit) override;
  void attack(int damage);
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
  float x, y;
  int spawner_id;
  bool triggered = false;
  Uint64 timer = 0;
};
