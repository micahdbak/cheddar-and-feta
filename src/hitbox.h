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
#define HB_SHARED_UNKNOWN THOOM_GID_MAX
#define HB_SHARED_FIRE (THOOM_GID_MAX - 1)
#define HB_SHARED_SPITTER (THOOM_GID_MAX - 2)

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
    thoom_gid shared_id = HB_SHARED_UNKNOWN;
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

  static std::string options(thoom_gid source_gid, thoom_gid hurtbox_owner_gid,
                             int x_off, int y_off, int w, int h,
                             Properties props) {
    char buff[256];
    snprintf(buff, sizeof(buff),
             "%" PRIu64 ",%" PRIu64 ",%d,%d,%d,%d,%d,%d,%d,%d,%" PRIu64 ",%d",
             source_gid, hurtbox_owner_gid, x_off, y_off, w, h, props.damage,
             props.cooldown_ms, props.delete_after_ms,
             props.shared_cooldowns ? 1 : 0, props.shared_id,
             props.single_use ? 1 : 0);
    return std::string(buff);
  }

  HitBox(thoom_gid source_gid, thoom_gid hurtbox_owner_gid, int x_off,
         int y_off, int w, int h, Properties props);
  ~HitBox();

  void step() override;
  void render() override;

  SDL_Rect bounding_box;

  bool throw_away = true;

  bool shared_cooldowns = false, single_use = false;
  thoom_gid shared_id = HB_SHARED_UNKNOWN;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  bool from_foe;
  thoom_gid source_gid, hurtbox_owner_gid;
  int x_off, y_off, damage, cooldown_ms, delete_after_ms;
  uint64_t created_ticks;

  SDL_FRect src_rect, dst_rect;
};
