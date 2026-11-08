#pragma once

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <string>

#include "object.h"
#include "scene/scene.h"
#include "thoom/macros.h"
#include "thoom/thoom.h"

#define HITBOX_OBJ "hitbox"

// hitbox shared IDs
#define HB_SHARED_UNKNOWN -1
#define HB_NO_OWNER ((thoom_lid) - 1)
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

  static std::string options(thoom_lid hitsource_owner_lid,
                             thoom_lid hurtbox_owner_lid, int x_off, int y_off,
                             int w, int h, Properties props) {
    char buff[256];
    snprintf(buff, sizeof(buff),
             "%" PRIu64 ",%" PRIu64 ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
             hitsource_owner_lid, hurtbox_owner_lid, x_off, y_off, w, h,
             props.damage, props.cooldown_ms, props.delete_after_ms,
             props.shared_cooldowns ? 1 : 0, props.shared_id,
             props.single_use ? 1 : 0);
    return std::string(buff);
  }

  HitBox(thoom_lid source_lid, thoom_lid hurtbox_owner_lid, int x_off,
         int y_off, int w, int h, Properties props);
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
  thoom_lid source_lid, hurtbox_owner_lid;
  int x_off, y_off, damage, cooldown_ms, delete_after_ms;
  uint64_t created_ticks;

  SDL_FRect src_rect, dst_rect;
};
