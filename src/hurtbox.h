#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <map>
#include <vector>

#include "hitbox.h"
#include "object/entity.h"
#include "object/object.h"

class HurtSource {
 public:
  HurtSource() = default;
  ~HurtSource() = default;

  virtual float hurtsource_x() = 0;
  virtual float hurtsource_y() = 0;
  virtual void attack(int damage) = 0;
};

class HurtBox : public thoom::Entity::Node {
 public:
  HurtBox(thoom::Object& owner, int x_off, int y_off, int w, int h);
  ~HurtBox();

  void hurt(int damage, HitBox* hitbox, int cooldown_ms);

  void step() override {}
  void render() override;
  bool update();

  SDL_Rect bounding_box;

  enum OwnerClass { MOUSE, FOE, HURTSOURCE } owner_class;

 private:
  int x_off, y_off;

  thoom::Object& owner;

  std::map<uint64_t, uint64_t> previous_hits;

  SDL_FRect src_rect, dst_rect;
};

extern std::vector<HurtBox*> hurtboxes;
extern uint64_t hurtbox_hitbox_key;
