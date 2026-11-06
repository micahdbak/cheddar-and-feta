#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

#include "object.h"
#include "scene.h"
#include "thoom/macros.h"

#define HITBOX_OBJ "hitbox"

// hitbox shared IDs
#define HB_SHARED_UNKNOWN -1
#define HB_SHARED_FIRE -2
#define HB_SHARED_SPITTER -3

class HitSource {
 public:
  HitSource() = default;
  ~HitSource() = default;

  virtual float hitsource_x() = 0;
  virtual float hitsource_y() = 0;
  virtual void hitsource_notify() {};
};

class HitBox : public thoom::Object {
 public:
  struct Properties {
    int damage;
    int cooldown_ms;
    int delete_after_ms = 0;
    bool shared_cooldowns = false;
    int shared_id = HB_SHARED_UNKNOWN;
    bool single_use = false;
  };

  static void make_offset(int x_dir, int y_dir, int* x_off, int* y_off,
                          float distance) {
    bool is_diagonal = x_dir != 0 && y_dir != 0;
    int hitbox_distance =
        (int)(is_diagonal ? THOOM_INV_SQRT2_F * distance : distance);
    *x_off = x_dir * hitbox_distance;
    *y_off = y_dir * hitbox_distance;
  }

  static std::string options(int hitsource_owner_id, int hurtbox_owner_id,
                             int x_off, int y_off, int w, int h,
                             Properties props) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
             hitsource_owner_id, hurtbox_owner_id, x_off, y_off, w, h,
             props.damage, props.cooldown_ms, props.delete_after_ms,
             props.shared_cooldowns ? 1 : 0, props.shared_id,
             props.single_use ? 1 : 0);
    return std::string(buff);
  }

  HitBox(int source_id, int hurtbox_owner_id, int x_off, int y_off, int w,
         int h, Properties props);
  ~HitBox();

  void step() override;

  SDL_Rect bounding_box;

  bool throw_away = true;

  bool shared_cooldowns = false, single_use = false;
  int shared_id = HB_SHARED_UNKNOWN;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  bool from_foe;
  int source_id, hurtbox_owner_id;
  int x_off, y_off, damage, cooldown_ms, delete_after_ms;
  uint64_t created_ticks;

  SDL_FRect src_rect, dst_rect;
};
