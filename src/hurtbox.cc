#include "hurtbox.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "foes/foe.h"
#include "hitbox.h"
#include "icon.h"
#include "mouse.h"
#include "object/entity.h"
#include "object/object.h"
#include "scene/scene.h"
#include "utils/utils.h"

#ifdef ONSCREEN_DEBUG
#include "renderer/renderer.h"
#include "window/window.h"
#endif

std::vector<HurtBox*> hurtboxes;
uint64_t hurtbox_hitbox_key;

HurtBox::HurtBox(thoom::Object& owner, int x_off, int y_off, int w, int h)
    : thoom::Entity::Node(owner, std::nullopt),
      owner(owner),
      x_off(x_off),
      y_off(y_off),
      bounding_box{0, 0, w, h} {
  if (dynamic_cast<HurtSource*>(&owner) != nullptr) {
    this->owner_class = HurtBox::OwnerClass::HURTSOURCE;
  } else if (dynamic_cast<Mouse*>(&owner) != nullptr) {
    this->owner_class = HurtBox::OwnerClass::MOUSE;
  } else if (dynamic_cast<Foe*>(&owner) != nullptr) {
    this->owner_class = HurtBox::OwnerClass::FOE;
  } else {
    this->deleted = true;
    return;
  }

  hurtboxes.push_back(this);
  this->src_rect = DEBUG_HURTBOX_ICON;
}

HurtBox::~HurtBox() {
  for (auto it = hurtboxes.begin(); it != hurtboxes.end(); it++) {
    if (*it == this) {
      hurtboxes.erase(it);
      break;
    }
  }
}

bool HurtBox::update() {
  switch (this->owner_class) {
    case HurtBox::OwnerClass::MOUSE: {
      Mouse* mouse = dynamic_cast<Mouse*>(&this->owner);
      if (mouse == nullptr) {
        return false;
      }
      this->bounding_box.x = (int)mouse->x + this->x_off;
      this->bounding_box.y = (int)mouse->y + this->y_off;
    } break;

    case HurtBox::OwnerClass::FOE: {
      Foe* foe = dynamic_cast<Foe*>(&this->owner);
      if (foe == nullptr || foe->state == Foe::State::DEAD) {
        return false;
      }
      this->bounding_box.x = (int)foe->x + (int)foe->off_x + this->x_off;
      this->bounding_box.y = (int)foe->y + (int)foe->off_y + this->y_off;
    } break;

    case HurtBox::OwnerClass::HURTSOURCE: {
      HurtSource* source = dynamic_cast<HurtSource*>(&this->owner);
      if (source == nullptr) {
        return false;
      }
      this->bounding_box.x = (int)source->hurtsource_x() + this->x_off;
      this->bounding_box.y = (int)source->hurtsource_y() + this->y_off;
    } break;
  }

  return true;
}

void HurtBox::hurt(int damage, HitBox* hitbox, int cooldown_ms) {
  bool should_hit = false;
  const uint64_t key = hitbox->cooldown_key();

  auto it = this->previous_hits.find(key);
  if (it == this->previous_hits.end()) {
    this->previous_hits[key] = thoom::Scene::instance->ticks;
    should_hit = true;
  } else if (thoom::Scene::instance->ticks - it->second > cooldown_ms) {
    this->previous_hits[key] = thoom::Scene::instance->ticks;
    should_hit = true;
  }

  if (!should_hit) {
    return;
  }

  hurtbox_hitbox_key = key;

  switch (this->owner_class) {
    case HurtBox::OwnerClass::MOUSE: {
      Mouse* mouse = dynamic_cast<Mouse*>(&this->owner);
      if (mouse == nullptr) return;
      mouse->attack(damage);

      int throw_x, throw_y;
      float hitbox_x =
          (float)(hitbox->bounding_box.x + hitbox->bounding_box.w / 2);
      float hitbox_y =
          (float)(hitbox->bounding_box.y + hitbox->bounding_box.h / 2);

      if (hitbox->throw_away) {
        thoom::dir_to_point(hitbox_x, hitbox_y, mouse->x, mouse->y, &throw_x,
                            &throw_y);
      } else {
        thoom::dir_to_point(mouse->x, mouse->y, hitbox_x, hitbox_y, &throw_x,
                            &throw_y);
      }

      mouse->set_throw(throw_x, throw_y);
    } break;

    case HurtBox::OwnerClass::FOE: {
      Foe* foe = dynamic_cast<Foe*>(&this->owner);
      if (foe == nullptr) return;
      foe->attack(damage);

      float hitbox_x =
          (float)(hitbox->bounding_box.x + hitbox->bounding_box.w / 2);
      float hitbox_y =
          (float)(hitbox->bounding_box.y + hitbox->bounding_box.h / 2);

      if (hitbox->throw_away) {
        thoom::dir_to_point(hitbox_x, hitbox_y, foe->x, foe->y, &foe->throw_x,
                            &foe->throw_y);
      } else {
        thoom::dir_to_point(foe->x, foe->y, hitbox_x, hitbox_y, &foe->throw_x,
                            &foe->throw_y);
      }
    } break;

    case HurtBox::OwnerClass::HURTSOURCE: {
      HurtSource* source = dynamic_cast<HurtSource*>(&this->owner);
      if (source != nullptr) {
        source->attack(damage);
      }
    } break;
  }
}

void HurtBox::render() {
  if (!this->update()) {
    return;
  }

#ifdef ONSCREEN_DEBUG
  this->dst_rect = {(float)this->bounding_box.x, (float)this->bounding_box.y,
                    (float)this->bounding_box.w, (float)this->bounding_box.h};
  thoom::Renderer::instance->push_sprite("sprites/icons.bmp", icons,
                                         &this->src_rect, &this->dst_rect,
                                         THOOM_SCREEN_HEIGHT);
#endif
}
