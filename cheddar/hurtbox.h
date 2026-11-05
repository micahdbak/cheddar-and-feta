#pragma once

#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "hitbox.h"
#include "object.h"
#include "scene.h"

#define HURTBOX_OBJ "hurtbox"

class HurtSource {
 public:
  HurtSource() = default;
  ~HurtSource() = default;

  virtual float hurtsource_x() = 0;
  virtual float hurtsource_y() = 0;
  virtual void attack(int damage) = 0;
};

/*
 * Main ideas:
 * - A hurt box represents a fixed-size rectangle, offset from the object that
 * created it
 * - Can be damaged with hurt(damage, hitbox_id, cooldown_ms) - this is done by
 * hitboxes
 * - Can only be hurt by a hitbox once within a cooldown
 */
class HurtBox : public thoom::Object {
 public:
  static std::string options(int owner_id, int x_off, int y_off, int w, int h) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d", owner_id, x_off, y_off, w,
             h);
    return std::string(buff);
  }

  HurtBox(int owner_id, int x_off, int y_off, int w, int h);
  ~HurtBox();

  void hurt(int damage, HitBox* hitbox, int cooldown_ms);

  void step() override;

  SDL_Rect bounding_box;

  int owner_id;
  enum OwnerClass { MOUSE, FOE, HURTSOURCE } owner_class;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int x_off, y_off;

  // maps a hitbox id to when it last hit (in ticks)
  std::map<int, uint64_t> previous_hits;

  SDL_FRect src_rect, dst_rect;
};

extern std::vector<HurtBox*> hurtboxes;

// can be used to check what hitbox attacked a given hurtbox
// (yes this is ugly, but I don't wanna change all attack() signatures to
// include the id cuz im lazy)
extern int hurtbox_hitbox_id;
