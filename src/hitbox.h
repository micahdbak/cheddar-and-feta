#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "object/area.h"
#include "object/entity.h"
#include "object/object.h"
#include "thoom/macros.h"
#include "thoom/thoom.h"
#include "utils/bytes.h"

#define COMBAT_EVENT_HIT 1u

constexpr uint32_t kSharedCooldownGroup = 1;

enum class HitType : uint32_t {
  kGeneric,
  kFire,
  kSpitter,
};

struct Hit {
  thoom::Entity::NodeKey source;
  thoom_gid instigator = THOOM_GID_INVALID;
  thoom_gid cooldown_scope = THOOM_GID_INVALID;
  uint32_t cooldown_group = 0;
  HitType type = HitType::kGeneric;
  int32_t damage = 0;
  uint32_t cooldown_ticks = 0;
  float contact_x = 0.0f;
  float contact_y = 0.0f;
  bool throw_away = true;

  void write(thoom::ByteWriter& writer) const;
  bool read(thoom::ByteReader& reader);
};

class HitSource {
 public:
  HitSource() = default;
  virtual ~HitSource() = default;

  virtual float hitsource_x() = 0;
  virtual float hitsource_y() = 0;
  virtual void hitsource_notify() {}
};

class HitBox : public thoom::Area {
 public:
  struct Properties {
    int32_t damage = 0;
    uint32_t cooldown_ticks = 0;
    uint32_t lifetime_ticks = 0;
    bool single_use = false;
    HitType type = HitType::kGeneric;
    thoom_gid cooldown_scope = THOOM_GID_INVALID;
    uint32_t cooldown_group = 0;
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

  bool throw_away = true;

 private:
  HitSource* source = nullptr;
  bool from_foe = false;
  bool single_use = false;
  uint32_t lifetime_ticks = 0;
  uint32_t age_ticks = 0;
  Hit hit;

  SDL_FRect src_rect, dst_rect;
};
