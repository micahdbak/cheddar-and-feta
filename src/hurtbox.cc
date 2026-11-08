#include "hurtbox.h"

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "foes/foe.h"
#include "hitbox.h"
#include "icon.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"
#include "thoom/thoom.h"
#include "utils/utils.h"

std::vector<HurtBox*> hurtboxes;
thoom_lid hurtbox_hitbox_lid;

HurtBox::HurtBox(thoom_lid owner_lid, int x_off, int y_off, int w, int h)
    : owner_lid(owner_lid),
      x_off(x_off),
      y_off(y_off),
      bounding_box{0, 0, w, h} {
  thoom::Object* owner = thoom::Scene::instance->find_object(owner_lid);
  if (owner == nullptr) return;

  HurtSource* source = dynamic_cast<HurtSource*>(owner);
  Mouse* mouse = dynamic_cast<Mouse*>(owner);
  Foe* foe = dynamic_cast<Foe*>(owner);

  if (source != nullptr) {
    this->owner_class = HurtBox::OwnerClass::HURTSOURCE;
    this->bounding_box.x = (int)source->hurtsource_y() + x_off;
    this->bounding_box.y = (int)source->hurtsource_x() + y_off;
  } else if (mouse != nullptr) {
    this->owner_class = HurtBox::OwnerClass::MOUSE;
    this->bounding_box.x = (int)mouse->x + x_off;
    this->bounding_box.y = (int)mouse->y + y_off;
  } else if (foe != nullptr) {
    this->owner_class = HurtBox::OwnerClass::FOE;
    this->bounding_box.x = (int)foe->x + x_off;
    this->bounding_box.y = (int)foe->y + y_off;
  } else {
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

void HurtBox::hurt(int damage, HitBox* hitbox, int cooldown_ms) {
  thoom::Object* owner = thoom::Scene::instance->find_object(this->owner_lid);
  if (owner == nullptr) return;

  bool should_hit = false;
  thoom_lid hitbox_lid =
      hitbox->shared_cooldowns ? (thoom_lid)hitbox->shared_id : hitbox->lid;

  auto it = this->previous_hits.find(hitbox_lid);
  if (it != this->previous_hits.end()) {
    uint64_t when_ticks = it->second;

    if (thoom::Scene::instance->ticks - when_ticks > cooldown_ms) {
      should_hit = true;
      this->previous_hits[hitbox_lid] = thoom::Scene::instance->ticks;
    }
  } else {
    // hitbox hasn't attacked this yet
    this->previous_hits[hitbox_lid] = thoom::Scene::instance->ticks;
    should_hit = true;
  }

  if (should_hit) {
    hurtbox_hitbox_lid = hitbox_lid;

    switch (this->owner_class) {
      case HurtBox::OwnerClass::MOUSE: {
        Mouse* mouse = dynamic_cast<Mouse*>(owner);
        if (mouse == nullptr) return;
        mouse->attack(damage);

        // throw mouse away from source
        int throw_x, throw_y;
        float hitbox_x =
            (float)(hitbox->bounding_box.x + hitbox->bounding_box.w / 2);
        float hitbox_y =
            (float)(hitbox->bounding_box.y + hitbox->bounding_box.h / 2);

        if (hitbox->throw_away) {
          // throws away from hitbox
          thoom::dir_to_point(hitbox_x, hitbox_y, mouse->x, mouse->y, &throw_x,
                              &throw_y);
        } else {
          // pulls toward hitbox
          thoom::dir_to_point(mouse->x, mouse->y, hitbox_x, hitbox_y, &throw_x,
                              &throw_y);
        }

        mouse->set_throw(throw_x, throw_y);
      } break;

      case HurtBox::OwnerClass::FOE: {
        Foe* foe = dynamic_cast<Foe*>(owner);
        if (foe == nullptr) return;
        foe->attack(damage);

        // throw mouse away from source
        float hitbox_x =
            (float)(hitbox->bounding_box.x + hitbox->bounding_box.w / 2);
        float hitbox_y =
            (float)(hitbox->bounding_box.y + hitbox->bounding_box.h / 2);

        if (hitbox->throw_away) {
          // throws away from hitbox
          thoom::dir_to_point(hitbox_x, hitbox_y, foe->x, foe->y, &foe->throw_x,
                              &foe->throw_y);
        } else {
          // pulls toward hitbox
          thoom::dir_to_point(foe->x, foe->y, hitbox_x, hitbox_y, &foe->throw_x,
                              &foe->throw_y);
        }
      } break;

      case HurtBox::OwnerClass::HURTSOURCE: {
        HurtSource* source = dynamic_cast<HurtSource*>(owner);
        if (source == nullptr) return;
        source->attack(damage);
      } break;
    }
  }
}

void HurtBox::step() {
  thoom::Object* owner = thoom::Scene::instance->find_object(this->owner_lid);

  if (owner == nullptr) {
    this->deleted = true;
    return;
  }

  // update bounding box
  switch (this->owner_class) {
    case HurtBox::OwnerClass::MOUSE: {
      Mouse* mouse = dynamic_cast<Mouse*>(owner);
      if (mouse == nullptr) {
        this->deleted = true;
        return;
      }
      this->bounding_box.x = (int)mouse->x + this->x_off;
      this->bounding_box.y = (int)mouse->y + this->y_off;
    } break;
    case HurtBox::OwnerClass::FOE: {
      Foe* foe = dynamic_cast<Foe*>(owner);
      if (foe == nullptr || foe->state == Foe::State::DEAD) {
        this->deleted = true;
        return;
      }
      this->bounding_box.x = (int)foe->x + (int)foe->off_x + this->x_off;
      this->bounding_box.y = (int)foe->y + (int)foe->off_y + this->y_off;
    } break;
    case HurtBox::OwnerClass::HURTSOURCE: {
      HurtSource* source = dynamic_cast<HurtSource*>(owner);
      if (source == nullptr) {
        this->deleted = true;
        return;
      }
      this->bounding_box.x = (int)source->hurtsource_x() + this->x_off;
      this->bounding_box.y = (int)source->hurtsource_y() + this->y_off;
    } break;
  }

#ifdef ONSCREEN_DEBUG
  this->dst_rect = {(float)this->bounding_box.x, (float)this->bounding_box.y,
                    (float)this->bounding_box.w, (float)this->bounding_box.h};
  thoom::Renderer::instance->push_sprite("sprites/icons.bmp", icons,
                                         &this->src_rect, &this->dst_rect,
                                         THOOM_SCREEN_HEIGHT);
#endif
}

thoom::Object* HurtBox::create_object(thoom::Scene* scene,
                                      const std::string& options) {
  thoom_lid owner_lid;
  int x_off, y_off, w, h;
  if (5 != sscanf(options.c_str(), "%" SCNu64 ",%d,%d,%d,%d", &owner_lid,
                  &x_off, &y_off, &w, &h)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new HurtBox(owner_lid, x_off, y_off, w, h);
}
