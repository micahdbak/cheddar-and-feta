#include "hitbox.h"

#include "hurtbox.h"

HitBox::HitBox(int hurtbox_owner_id, HitSource* source,
               std::shared_ptr<bool> source_deleted, int x_off, int y_off,
               int w, int h, HitBox::Properties props)
    : bounding_box{0, 0, w, h},
      hurtbox_owner_id(hurtbox_owner_id),
      source(source),
      source_deleted(source_deleted),
      x_off(x_off),
      y_off(y_off),
      damage(props.damage),
      cooldown_ms(props.cooldown_ms),
      delete_after_ms(props.delete_after_ms),
      shared_cooldowns(props.shared_cooldowns),
      shared_id(props.shared_id),
      single_use(props.single_use) {
  this->created_ticks = game->ticks;
  this->src_rect = DEBUG_HITBOX_ICON;

  // if not from a foe, hit all hurtboxes
  this->from_foe = false;

  if (hurtbox_owner_id != -1) {
    // if from a foe, hit only mice
    Object* owner_obj = game->get_object(hurtbox_owner_id);
    Foe* foe;
    if ((foe = dynamic_cast<Foe*>(owner_obj)) != nullptr) {
      this->from_foe = true;
    }
  }
}

HitBox::~HitBox() {
  // do nothing
}

void HitBox::step() {
  if (*this->source_deleted ||
      (this->delete_after_ms > 0 &&
       game->ticks - this->created_ticks > this->delete_after_ms)) {
    game->delete_object = true;
    return;
  }

  this->bounding_box.x = (int)this->source->hitsource_x() + this->x_off;
  this->bounding_box.y = (int)this->source->hitsource_y() + this->y_off;

  for (HurtBox* hb : hurtboxes) {
    if (this->from_foe && hb->owner_class == HurtBox::OwnerClass::FOE) {
      continue;  // don't hit other foes
    }

    if (this->hurtbox_owner_id != hb->owner_id &&
        SDL_HasRectIntersection(&this->bounding_box, &hb->bounding_box)) {
      hb->hurt(this->damage, this, this->cooldown_ms);
      this->source->hitsource_notify();

      if (this->single_use) {
        // will be deleted on next loop
        this->created_ticks = game->ticks - (this->delete_after_ms + 1000);
        break;  // end for loop
      }
    }
  }

#ifdef ONSCREEN_DEBUG
  this->dst_rect = {(float)this->bounding_box.x, (float)this->bounding_box.y,
                    (float)this->bounding_box.w, (float)this->bounding_box.h};
  game->push_sprite("sprites/icons.bmp", game->icons, &this->src_rect,
                    &this->dst_rect, SCREEN_HEIGHT);
#endif
}
