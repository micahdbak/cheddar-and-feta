#pragma once

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "hitbox.h"
#include "object.h"
#include "scene/scene.h"
#include "thoom/thoom.h"

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
 * - Can be damaged with hurt(damage, hitbox, cooldown_ms) - this is done by
 * hitboxes
 * - Can only be hurt by a hitbox once within a cooldown
 */
class HurtBox : public thoom::Object {
 public:
  static std::string options(thoom_gid owner_gid, int x_off, int y_off, int w,
                             int h) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%" PRIu64 ",%d,%d,%d,%d", owner_gid, x_off,
             y_off, w, h);
    return std::string(buff);
  }

  HurtBox(thoom_gid owner_gid, int x_off, int y_off, int w, int h);
  ~HurtBox();

  void hurt(int damage, HitBox* hitbox, int cooldown_ms);

  void step() override;
  void render() override;

  SDL_Rect bounding_box;

  thoom_gid owner_gid;
  enum OwnerClass { MOUSE, FOE, HURTSOURCE } owner_class;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int x_off, y_off;

  // maps a hitbox id to when it last hit (in ticks)
  std::map<thoom_gid, uint64_t> previous_hits;

  SDL_FRect src_rect, dst_rect;
};

extern std::vector<HurtBox*> hurtboxes;

// can be used to check what hitbox attacked a given hurtbox
// (yes this is ugly, but I don't wanna change all attack() signatures to
// include the id cuz im lazy)
extern thoom_gid hurtbox_hitbox_gid;
