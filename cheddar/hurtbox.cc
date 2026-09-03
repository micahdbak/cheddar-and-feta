#include "hurtbox.h"

#include "foes/foe.h"
#include "game.h"
#include "mouse.h"

std::vector<HurtBox*> hurtboxes;
int hurtbox_hitbox_id;

HurtBox::HurtBox(int owner_id, int x_off, int y_off, int w, int h)
    : owner_id(owner_id), x_off(x_off), y_off(y_off), bounding_box{0, 0, w, h} {
  this->owner = game->get_object(owner_id);
  if (this->owner == nullptr) return;

  if ((this->source = dynamic_cast<HurtSource*>(this->owner)) != nullptr) {
    this->owner_class = HurtBox::OwnerClass::HURTSOURCE;
    this->bounding_box.x = (int)this->source->hurtsource_y() + x_off;
    this->bounding_box.y = (int)this->source->hurtsource_x() + y_off;
  } else if ((this->mouse = dynamic_cast<Mouse*>(this->owner)) != nullptr) {
    this->owner_class = HurtBox::OwnerClass::MOUSE;
    this->bounding_box.x = (int)this->mouse->x + x_off;
    this->bounding_box.y = (int)this->mouse->y + y_off;
  } else if ((this->foe = dynamic_cast<Foe*>(this->owner)) != nullptr) {
    this->owner_class = HurtBox::OwnerClass::FOE;
    this->bounding_box.x = (int)this->foe->x + x_off;
    this->bounding_box.y = (int)this->foe->y + y_off;
  } else {
    this->owner = nullptr;  // owner must be a Mouse, Foe, or a HurtSource
  }

  // owner is Mouse or Foe
  if (this->owner != nullptr) {
    this->owner_deleted = this->owner->get_deleted_ptr();
    hurtboxes.push_back(this);
  }

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
  // don't go through with a hit if the owner of the hurtbox was deleted anyways
  if (owner_deleted == nullptr || *this->owner_deleted) {
    this->owner = nullptr;
    return;
  }

  bool should_hit = false;
  int hitbox_id = hitbox->shared_cooldowns ? hitbox->shared_id : hitbox->id;

  auto it = this->previous_hits.find(hitbox_id);
  if (it != this->previous_hits.end()) {
    uint64_t when_ticks = it->second;

    if (game->ticks - when_ticks > cooldown_ms) {
      should_hit = true;
      this->previous_hits[hitbox_id] = game->ticks;
    }
  } else {
    // hitbox hasn't attacked this yet
    this->previous_hits[hitbox_id] = game->ticks;
    should_hit = true;
  }

  if (should_hit) {
    hurtbox_hitbox_id = hitbox_id;

    switch (this->owner_class) {
      case HurtBox::OwnerClass::MOUSE: {
        this->mouse->attack(damage);

        // throw mouse away from source
        int throw_x, throw_y;
        float hitbox_x =
            (float)(hitbox->bounding_box.x + hitbox->bounding_box.w / 2);
        float hitbox_y =
            (float)(hitbox->bounding_box.y + hitbox->bounding_box.h / 2);

        if (hitbox->throw_away) {
          // throws away from hitbox
          dir_to_point(hitbox_x, hitbox_y, this->mouse->x, this->mouse->y,
                       &throw_x, &throw_y);
        } else {
          // pulls toward hitbox
          dir_to_point(this->mouse->x, this->mouse->y, hitbox_x, hitbox_y,
                       &throw_x, &throw_y);
        }

        this->mouse->set_throw(throw_x, throw_y);
      } break;

      case HurtBox::OwnerClass::FOE: {
        this->foe->attack(damage);

        // throw mouse away from source
        float hitbox_x =
            (float)(hitbox->bounding_box.x + hitbox->bounding_box.w / 2);
        float hitbox_y =
            (float)(hitbox->bounding_box.y + hitbox->bounding_box.h / 2);

        if (hitbox->throw_away) {
          // throws away from hitbox
          dir_to_point(hitbox_x, hitbox_y, this->foe->x, this->foe->y,
                       &this->foe->throw_x, &this->foe->throw_y);
        } else {
          // pulls toward hitbox
          dir_to_point(this->foe->x, this->foe->y, hitbox_x, hitbox_y,
                       &this->foe->throw_x, &this->foe->throw_y);
        }
      } break;

      case HurtBox::OwnerClass::HURTSOURCE:
        this->source->attack(damage);
        break;
    }
  }
}

void HurtBox::step() {
  if (this->owner == nullptr || this->owner_deleted == nullptr ||
      *this->owner_deleted ||
      (this->owner_class == HurtBox::OwnerClass::FOE &&
       this->foe->state == Foe::State::DEAD)) {
    game->delete_object = true;
    return;
  }

  // update bounding box
  switch (this->owner_class) {
    case HurtBox::OwnerClass::MOUSE:
      this->bounding_box.x = (int)this->mouse->x + this->x_off;
      this->bounding_box.y = (int)this->mouse->y + this->y_off;
      break;
    case HurtBox::OwnerClass::FOE:
      this->bounding_box.x =
          (int)this->foe->x + (int)this->foe->off_x + this->x_off;
      this->bounding_box.y =
          (int)this->foe->y + (int)this->foe->off_y + this->y_off;
      break;
    case HurtBox::OwnerClass::HURTSOURCE:
      this->bounding_box.x = (int)this->source->hurtsource_x() + this->x_off;
      this->bounding_box.y = (int)this->source->hurtsource_y() + this->y_off;
      break;
  }

#ifdef ONSCREEN_DEBUG
  this->dst_rect = {(float)this->bounding_box.x, (float)this->bounding_box.y,
                    (float)this->bounding_box.w, (float)this->bounding_box.h};
  game->push_sprite("sprites/icons.bmp", game->icons, &this->src_rect,
                    &this->dst_rect, SCREEN_HEIGHT);
#endif
}
