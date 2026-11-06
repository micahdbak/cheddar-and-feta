#include "cannon_ball.h"

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

ThrownCannonBall::ThrownCannonBall(float x, float y, int x_dir, int y_dir,
                                   int from_id)
    : x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
  // int dx, dy;
  // if (this->x_dir != 0 && this->y_dir != 0) {
  //     dx = this->x_dir * 12;
  //     dy = this->y_dir * 12;
  // } else {
  //     dx = this->x_dir * 16;
  //     dy = this->y_dir * 16;
  // }
  //
  // this->x += (float)dx;
  // this->y += (float)dy;

  thoom::Object* obj = thoom::Scene::instance->find_object(from_id);
  Mouse* mouse;
  if (obj != nullptr && (mouse = dynamic_cast<Mouse*>(obj)) != nullptr) {
    // only drop if thrown by a mouse
    this->drop_item = true;
  } else {
    this->drop_item = false;
  }

  this->did_hit = false;

  this->sprite = new thoom::Sprite("sprites/item_cannon_ball.bmp", 16, 16, 0);
  this->dst_rect.w = 16.0f;
  this->dst_rect.h = 16.0f;

  this->spawned_ticks = thoom::Scene::instance->ticks;

  HitBox::Properties props = {2, 1000, 1000};
  props.single_use = true;
  thoom::Scene::instance->create_object(
      HITBOX_OBJ, HitBox::options(this->id, from_id, -8, -8, 16, 16, props));
}

ThrownCannonBall::~ThrownCannonBall() {
  delete this->sprite;
  this->sprite = nullptr;
}

void ThrownCannonBall::step() {
  float dx = float(this->x_dir) *
             (this->y_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * 64.0f *
             thoom::Scene::instance->delta;
  float dy = float(this->y_dir) *
             (this->x_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * 64.0f *
             thoom::Scene::instance->delta;
  float new_x = this->x + dx;
  float new_y = this->y + dy;

  if (thoom::Scene::instance->map->collider->point_in_collider(new_x, new_y) ||
      thoom::Scene::instance->ticks - this->spawned_ticks > 750) {
    this->deleted = true;

    if (this->drop_item) {
      thoom::Scene::instance->create_object(
          ITEM_CANNON_BALL DROPPED_OBJ, DroppedItem::options(this->x, this->y));
    }

    return;
  }

  this->x = new_x;
  this->y = new_y;

  this->dst_rect.x = float(this->x - 8);
  this->dst_rect.y = float(this->y - 8);

  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 16);
}

void ThrownCannonBall::hitsource_notify() {
  this->did_hit = true;
  this->spawned_ticks =
      thoom::Scene::instance->ticks - 500;  // 250ms until dropped
}

thoom::Object* DroppedCannonBall::create_object(thoom::Scene* scene,
                                                const std::string& options) {
  int x, y;
  if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new DroppedCannonBall(float(x), float(y));
}

thoom::Object* ThrownCannonBall::create_object(thoom::Scene* scene,
                                               const std::string& options) {
  int x, y, x_dir, y_dir, from_id;
  if (5 != sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir,
                  &from_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new ThrownCannonBall((float)x, (float)y, x_dir, y_dir, from_id);
}
