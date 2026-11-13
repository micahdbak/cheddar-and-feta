#include "hitbox.h"

#include <cstdint>
#include <optional>

#include "foes/foe.h"
#include "hurtbox.h"
#include "icon.h"
#include "object/entity.h"
#include "object/object.h"
#include "scene/scene.h"

#ifdef ONSCREEN_DEBUG
#include "renderer/renderer.h"
#include "window/window.h"
#endif

HitBox::HitBox(thoom::Object& source, thoom::Entity* instigator, int x_off,
               int y_off, int w, int h, HitBox::Properties props)
    : thoom::Entity::Node(source, std::nullopt),
      bounding_box{0, 0, w, h},
      shared_cooldowns(props.shared_cooldowns),
      single_use(props.single_use),
      shared_id(props.shared_id),
      source(dynamic_cast<HitSource*>(&source)),
      instigator(instigator),
      from_foe(dynamic_cast<Foe*>(instigator) != nullptr),
      x_off(x_off),
      y_off(y_off),
      damage(props.damage),
      cooldown_ms(props.cooldown_ms),
      delete_after_ms(props.delete_after_ms) {
  if (this->source == nullptr) {
    this->deleted = true;
    return;
  }

  this->created_ticks = thoom::Scene::instance->ticks;
  this->src_rect = DEBUG_HITBOX_ICON;
}

void HitBox::step() {
  if (this->source == nullptr ||
      (this->delete_after_ms > 0 &&
       thoom::Scene::instance->ticks - this->created_ticks >
           this->delete_after_ms)) {
    this->deleted = true;
    return;
  }

  this->bounding_box.x = (int)source->hitsource_x() + this->x_off;
  this->bounding_box.y = (int)source->hitsource_y() + this->y_off;

  for (HurtBox* hb : hurtboxes) {
    if (this->from_foe && hb->owner_class == HurtBox::OwnerClass::FOE) {
      continue;
    }

    if (this->instigator != &hb->entity && hb->update() &&
        SDL_HasRectIntersection(&this->bounding_box, &hb->bounding_box)) {
      hb->hurt(this->damage, this, this->cooldown_ms);
      this->source->hitsource_notify();

      if (this->single_use) {
        this->created_ticks =
            thoom::Scene::instance->ticks - (this->delete_after_ms + 1000);
        break;
      }
    }
  }
}

uint64_t HitBox::cooldown_key() const {
  return this->shared_cooldowns ? this->shared_id : this->lid;
}

void HitBox::render() {
#ifdef ONSCREEN_DEBUG
  this->dst_rect = {(float)this->bounding_box.x, (float)this->bounding_box.y,
                    (float)this->bounding_box.w, (float)this->bounding_box.h};
  thoom::Renderer::instance->push_sprite("sprites/icons.bmp", icons,
                                         &this->src_rect, &this->dst_rect,
                                         THOOM_SCREEN_HEIGHT);
#endif
}
