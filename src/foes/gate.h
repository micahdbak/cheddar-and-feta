#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#include "object/entity.h"
#include "sprite/sprite.h"

#define GATE_OBJ "gate"

class FoeGate : public thoom::Entity {
 public:
  FoeGate(const thoom::Entity::Context& context, int x, int y, int spawner_id,
          int animation);
  ~FoeGate();

  void step() override;
  void render() override;
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  float x, y;
  int colliders_offset, colliders_x, spawner_id;
  bool opened = false;

  thoom::Sprite* sprite;
  SDL_FRect dst_rect;

  std::vector<int> colliders;

  bool synchronized = false;
};
