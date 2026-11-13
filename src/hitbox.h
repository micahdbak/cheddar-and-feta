#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "object/entity.h"
#include "object/object.h"
#include "thoom/macros.h"

#define HB_SHARED_NONE (UINT64_MAX - 1)
#define HB_SHARED_FIRE (UINT64_MAX - 2)
#define HB_SHARED_SPITTER (UINT64_MAX - 3)

class HitSource {
 public:
  HitSource() = default;
  ~HitSource() = default;

  virtual float hitsource_x() = 0;
  virtual float hitsource_y() = 0;
  virtual void hitsource_notify() {};
};

class HitBox : public thoom::Entity::Node {
 public:
  struct Properties {
    int damage;
    int cooldown_ms;
    int delete_after_ms = 0;
    bool shared_cooldowns = false;
    uint64_t shared_id = HB_SHARED_NONE;
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

  HitBox(thoom::Object& source, thoom::Entity* instigator, int x_off, int y_off,
         int w, int h, Properties props);
  ~HitBox() = default;

  void step() override;
  void render() override;
  uint64_t cooldown_key() const;

  SDL_Rect bounding_box;

  bool throw_away = true;

  bool shared_cooldowns = false, single_use = false;
  uint64_t shared_id = HB_SHARED_NONE;

 private:
  HitSource* source = nullptr;
  thoom::Entity* instigator = nullptr;
  bool from_foe = false;
  int x_off, y_off, damage, cooldown_ms, delete_after_ms;
  uint64_t created_ticks;

  SDL_FRect src_rect, dst_rect;
};
