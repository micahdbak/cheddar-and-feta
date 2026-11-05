#include "hitbox.h"

#include "hurtbox.h"
#include "icon.h"
#include "renderer.h"
#include "window.h"

HitBox::HitBox(int source_id, int hurtbox_owner_id, int x_off, int y_off, int w,
               int h, HitBox::Properties props)
    : bounding_box{0, 0, w, h},
      source_id(source_id),
      hurtbox_owner_id(hurtbox_owner_id),
      x_off(x_off),
      y_off(y_off),
      damage(props.damage),
      cooldown_ms(props.cooldown_ms),
      delete_after_ms(props.delete_after_ms),
      shared_cooldowns(props.shared_cooldowns),
      shared_id(props.shared_id),
      single_use(props.single_use) {
  this->created_ticks = thoom::Scene::instance->ticks;
  this->src_rect = DEBUG_HITBOX_ICON;

  // if not from a foe, hit all hurtboxes
  this->from_foe = false;

  if (hurtbox_owner_id != -1) {
    // if from a foe, hit only mice
    thoom::Object* owner_obj =
        thoom::Scene::instance->find_object(hurtbox_owner_id);
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
  thoom::Object* source_object =
      thoom::Scene::instance->find_object(this->source_id);
  HitSource* source = dynamic_cast<HitSource*>(source_object);

  if (source == nullptr ||
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
      continue;  // don't hit other foes
    }

    if (this->hurtbox_owner_id != hb->owner_id &&
        SDL_HasRectIntersection(&this->bounding_box, &hb->bounding_box)) {
      hb->hurt(this->damage, this, this->cooldown_ms);
      source->hitsource_notify();

      if (this->single_use) {
        // will be deleted on next loop
        this->created_ticks =
            thoom::Scene::instance->ticks - (this->delete_after_ms + 1000);
        break;  // end for loop
      }
    }
  }

#ifdef ONSCREEN_DEBUG
  this->dst_rect = {(float)this->bounding_box.x, (float)this->bounding_box.y,
                    (float)this->bounding_box.w, (float)this->bounding_box.h};
  thoom::Renderer::instance->push_sprite("sprites/icons.bmp", icons,
                                         &this->src_rect, &this->dst_rect,
                                         THOOM_SCREEN_HEIGHT);
#endif
}

thoom::Object* HitBox::create_object(thoom::Scene* scene,
                                     const std::string& options) {
  int hitsource_owner_id, hurtbox_owner_id, x_off, y_off, w, h, damage,
      cooldown_ms, delete_after_ms, shared_cooldowns, shared_id, single_use;
  if (12 != sscanf(options.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                   &hitsource_owner_id, &hurtbox_owner_id, &x_off, &y_off, &w,
                   &h, &damage, &cooldown_ms, &delete_after_ms,
                   &shared_cooldowns, &shared_id, &single_use)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  HitBox::Properties props;
  props.damage = damage;
  props.cooldown_ms = cooldown_ms;
  props.delete_after_ms = delete_after_ms;
  props.shared_cooldowns = shared_cooldowns == 1;
  props.shared_id = shared_id;
  props.single_use = single_use == 1;

  return new HitBox(hitsource_owner_id, hurtbox_owner_id, x_off, y_off, w, h,
                    props);
}
