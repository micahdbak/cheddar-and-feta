#include "hitbox.h"

#include <cstdint>
#include <optional>
#include <utility>

#include "foes/foe.h"
#include "hurtbox.h"
#include "icon.h"
#include "object/area.h"
#include "object/entity.h"
#include "object/object.h"
#include "scene/scene.h"
#include "thoom/log.h"
#include "thoom/thoom.h"
#include "utils/bytes.h"

#ifdef ONSCREEN_DEBUG
#include "renderer/renderer.h"
#include "window/window.h"
#endif

void Hit::write(thoom::ByteWriter& writer) const {
  writer.write_u64(this->source.gid);
  writer.write_u32(this->source.nid);
  writer.write_u64(this->instigator);
  writer.write_u64(this->cooldown_scope);
  writer.write_u32(this->cooldown_group);
  writer.write_u32(static_cast<uint32_t>(this->type));
  writer.write_u32(static_cast<uint32_t>(this->damage));
  writer.write_u32(this->cooldown_ticks);
  writer.write_float(this->contact_x);
  writer.write_float(this->contact_y);
  writer.write_bool(this->throw_away);
}

bool Hit::read(thoom::ByteReader& reader) {
  this->source.gid = reader.read_u64();
  this->source.nid = reader.read_u32();
  this->instigator = reader.read_u64();
  this->cooldown_scope = reader.read_u64();
  this->cooldown_group = reader.read_u32();
  this->type = static_cast<HitType>(reader.read_u32());
  this->damage = static_cast<int32_t>(reader.read_u32());
  this->cooldown_ticks = reader.read_u32();
  this->contact_x = reader.read_float();
  this->contact_y = reader.read_float();
  this->throw_away = reader.read_bool();
  return reader.ok() && reader.remaining() == 0;
}

HitBox::HitBox(thoom::Object& source, thoom::Entity* instigator, int x_off,
               int y_off, int w, int h, HitBox::Properties props)
    : thoom::Area(source, std::nullopt,
                  SDL_FRect{float(x_off), float(y_off), float(w), float(h)}, 0),
      source(dynamic_cast<HitSource*>(&source)),
      from_foe(dynamic_cast<Foe*>(instigator) != nullptr),
      single_use(props.single_use),
      lifetime_ticks(props.lifetime_ticks) {
  if (this->source == nullptr) {
    thoom_log_warn("HitBox: parent is not a HitSource");
    this->deleted = true;
    return;
  }

  this->hit.instigator =
      instigator != nullptr ? instigator->gid : THOOM_GID_INVALID;
  this->hit.cooldown_scope = props.cooldown_scope;
  this->hit.cooldown_group = props.cooldown_group;
  this->hit.type = props.type;
  this->hit.damage = props.damage;
  this->hit.cooldown_ticks = props.cooldown_ticks;
  this->src_rect = DEBUG_HITBOX_ICON;
}

void HitBox::step() {
  if (this->source == nullptr ||
      (this->lifetime_ticks > 0 && this->age_ticks >= this->lifetime_ticks)) {
    this->deleted = true;
    return;
  }

  this->age_ticks++;
  this->origin_x = this->source->hitsource_x();
  this->origin_y = this->source->hitsource_y();
  this->hit.source = this->key();
  this->hit.throw_away = this->throw_away;

  const SDL_FRect bounds = this->bounds();
  this->hit.contact_x = bounds.x + bounds.w / 2.0f;
  this->hit.contact_y = bounds.y + bounds.h / 2.0f;

  thoom::AreaQuery query;
  query.bounds = bounds;
  query.groups = HURT_GROUPS_ALL;
  query.authority = thoom::Entity::AuthorityScope::kAll;

  for (thoom::Area* area : thoom::Scene::instance->find_areas(query)) {
    if (this->hit.instigator == area->entity.gid ||
        (this->from_foe && (area->groups & HURT_GROUP_FOE) != 0)) {
      continue;
    }

    thoom::Entity::Event event;
    event.target = area->key();
    event.kind = COMBAT_EVENT_HIT;
    thoom::ByteWriter writer(event.payload);
    this->hit.write(writer);
    thoom::Scene::instance->queue_event(std::move(event));

    this->source->hitsource_notify();
    if (this->single_use) {
      this->deleted = true;
      return;
    }
  }
}

void HitBox::render() {
#ifdef ONSCREEN_DEBUG
  this->dst_rect = this->bounds();
  thoom::Renderer::instance->push_sprite("sprites/icons.bmp", icons,
                                         &this->src_rect, &this->dst_rect,
                                         THOOM_SCREEN_HEIGHT);
#endif
}
