#include "toothpick.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "hitbox.h"
#include "item.h"
#include "mouse.h"
#include "object.h"
#include "renderer.h"
#include "scene.h"
#include "sprite.h"
#include "thoom/macros.h"
#include "utils.h"

ThrownToothpick::ThrownToothpick(float x, float y, int x_dir, int y_dir,
                                 int from_id)
    : x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
  thoom::Object* obj = thoom::Scene::instance->find_object(from_id);
  Mouse* mouse;
  if (obj != nullptr && (mouse = dynamic_cast<Mouse*>(obj)) != nullptr) {
    // only drop if thrown by a mouse
    this->drop_item = true;
  } else {
    this->drop_item = false;
  }

  this->did_hit = false;

  this->sprite = new thoom::Sprite("sprites/item_toothpick.bmp", 16, 16, 0);
  this->sprite->set_animation(thoom::direction_from_dirs(x_dir, y_dir));
  this->dst_rect.w = 16.0f;
  this->dst_rect.h = 16.0f;

  this->spawned_ticks = thoom::Scene::instance->ticks;

  HitBox::Properties props = {1, 1000, 1000};
  props.single_use = true;
  thoom::Scene::instance->create_object(
      HITBOX_OBJ, HitBox::options(this->id, from_id, -8, -8, 16, 16, props));
}

ThrownToothpick::~ThrownToothpick() {
  delete this->sprite;
  this->sprite = nullptr;
}

void ThrownToothpick::step() {
  float dx = float(this->x_dir) *
             (this->y_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * 128.0f *
             thoom::Scene::instance->delta;
  float dy = float(this->y_dir) *
             (this->x_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * 128.0f *
             thoom::Scene::instance->delta;
  float new_x = this->x + dx;
  float new_y = this->y + dy;

  if (thoom::Scene::instance->map->collider->point_in_collider(new_x, new_y) ||
      thoom::Scene::instance->ticks - this->spawned_ticks > 1000) {
    this->deleted = true;

    if (this->drop_item) {
      thoom::Scene::instance->create_object(
          ITEM_TOOTHPICK DROPPED_OBJ, DroppedItem::options(this->x, this->y));
    }

    return;
  }

  this->x = new_x;
  this->y = new_y;

  this->dst_rect.x = this->x - 8.0f;
  this->dst_rect.y = this->y - 8.0f;

  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 16);
}

void ThrownToothpick::hitsource_notify() {
  this->did_hit = true;
  this->spawned_ticks =
      thoom::Scene::instance->ticks - 750;  // 250ms until dropped
}

thoom::Object* DroppedToothpick::create_object(thoom::Scene* scene,
                                               const std::string& options) {
  int x, y;
  if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new DroppedToothpick(float(x), float(y));
}

thoom::Object* ThrownToothpick::create_object(thoom::Scene* scene,
                                              const std::string& options) {
  int x, y, x_dir, y_dir, from_id;
  if (5 != sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir,
                  &from_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new ThrownToothpick((float)x, (float)y, x_dir, y_dir, from_id);
}
