#pragma once

#include <vector>

#include "object.h"
#include "scene.h"
#include "sprite.h"

#define GATE_OBJ "gate"

class FoeGate : public thoom::Object {
 public:
  FoeGate(int x, int y, int spawner_id, int animation);
  ~FoeGate();

  void step() override;
  static thoom::Object* create_object(thoom::Scene* scene,
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
