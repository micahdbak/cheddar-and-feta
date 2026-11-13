#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <map>
#include <optional>
#include <utility>

#include "hitbox.h"
#include "object/area.h"
#include "object/entity.h"
#include "object/object.h"
#include "thoom/thoom.h"

#define HURT_GROUP_MOUSE (1u << 0)
#define HURT_GROUP_FOE (1u << 1)
#define HURT_GROUP_WORLD (1u << 2)
#define HURT_GROUPS_ALL (HURT_GROUP_MOUSE | HURT_GROUP_FOE | HURT_GROUP_WORLD)

constexpr thoom_nid kBodyHurtboxNid = 1;

class HitReceiver {
 public:
  HitReceiver() = default;
  virtual ~HitReceiver() = default;

  virtual float hurt_x() const = 0;
  virtual float hurt_y() const = 0;
  virtual bool hittable() const { return true; }
  virtual void receive_hit(const Hit& hit) = 0;
};

class HurtBox : public thoom::Area {
 public:
  HurtBox(thoom::Object& owner, std::optional<thoom_nid> explicit_nid,
          uint32_t groups, int x_off, int y_off, int w, int h);
  ~HurtBox() = default;

  void step() override;
  void render() override;
  void receive_event(const thoom::Entity::Event& event) override;

 private:
  typedef std::pair<thoom_gid, uint32_t> SharedCooldown;

  bool accept_hit(const Hit& hit);
  void expire_cooldowns();

  HitReceiver* receiver = nullptr;

  std::map<thoom::Entity::NodeKey, uint32_t> previous_hits;
  std::map<SharedCooldown, uint32_t> previous_shared_hits;

  SDL_FRect src_rect, dst_rect;
};
