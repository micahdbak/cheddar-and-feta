#include "hurtbox.h"

#include <cstdint>
#include <map>
#include <optional>
#include <utility>

#include "hitbox.h"
#include "icon.h"
#include "object/area.h"
#include "object/entity.h"
#include "object/object.h"
#include "thoom/log.h"
#include "thoom/thoom.h"
#include "utils/bytes.h"

#ifdef ONSCREEN_DEBUG
#include "renderer/renderer.h"
#include "window/window.h"
#endif

HurtBox::HurtBox(thoom::Object& owner, std::optional<thoom_nid> explicit_nid,
                 uint32_t groups, int x_off, int y_off, int w, int h)
    : thoom::Area(owner, explicit_nid,
                  SDL_FRect{float(x_off), float(y_off), float(w), float(h)},
                  groups),
      receiver(dynamic_cast<HitReceiver*>(&owner)) {
  if (this->receiver == nullptr) {
    thoom_log_warn("HurtBox: owner is not a HitReceiver");
    this->deleted = true;
    return;
  }

  this->src_rect = DEBUG_HURTBOX_ICON;
}

void HurtBox::step() {
  this->expire_cooldowns();

  if (this->receiver == nullptr || !this->receiver->hittable()) {
    this->enabled = false;
    return;
  }

  this->enabled = true;
  this->origin_x = this->receiver->hurt_x();
  this->origin_y = this->receiver->hurt_y();
}

void HurtBox::render() {
  if (!this->enabled) {
    return;
  }

#ifdef ONSCREEN_DEBUG
  this->dst_rect = this->bounds();
  thoom::Renderer::instance->push_sprite("sprites/icons.bmp", icons,
                                         &this->src_rect, &this->dst_rect,
                                         THOOM_SCREEN_HEIGHT);
#endif
}

void HurtBox::receive_event(const thoom::Entity::Event& event) {
  if (event.kind != COMBAT_EVENT_HIT ||
      (this->receiver != nullptr && !this->receiver->hittable())) {
    return;
  }

  thoom::ByteReader reader(event.payload);
  Hit hit;
  if (!hit.read(reader)) {
    thoom_log_warn("HurtBox: malformed hit event");
    return;
  }

  if (this->accept_hit(hit)) {
    this->receiver->receive_hit(hit);
  }
}

bool HurtBox::accept_hit(const Hit& hit) {
  if (hit.cooldown_ticks == 0) {
    return true;
  }

  if (hit.cooldown_group != 0) {
    const SharedCooldown key = {hit.cooldown_scope, hit.cooldown_group};
    return this->previous_shared_hits.emplace(key, hit.cooldown_ticks).second;
  }

  return this->previous_hits.emplace(hit.source, hit.cooldown_ticks).second;
}

void HurtBox::expire_cooldowns() {
  for (auto it = this->previous_hits.begin();
       it != this->previous_hits.end();) {
    if (it->second <= 1) {
      it = this->previous_hits.erase(it);
    } else {
      it->second--;
      it++;
    }
  }

  for (auto it = this->previous_shared_hits.begin();
       it != this->previous_shared_hits.end();) {
    if (it->second <= 1) {
      it = this->previous_shared_hits.erase(it);
    } else {
      it->second--;
      it++;
    }
  }
}
