#include "cannon_ball.h"

#include <iostream>

#include "item.h"
#include "utils.h"

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

  thoom::Object* obj = thoom::game->get_object(from_id);
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

  this->spawned_ticks = thoom::game->ticks;

  HitBox::Properties props = {2, 1000, 1000};
  props.single_use = true;
  thoom::game->push_object(
      HITBOX_OBJ, HitBox::Options(this->id, from_id, -8, -8, 16, 16, props));
}

ThrownCannonBall::~ThrownCannonBall() {
  delete this->sprite;
  this->sprite = nullptr;
}

void ThrownCannonBall::step() {
  float dx = float(this->x_dir) *
             (this->y_dir != 0 ? THOOM_DIAG_MULTIPLIER : 1.0f) * 64.0f *
             thoom::game->delta;
  float dy = float(this->y_dir) *
             (this->x_dir != 0 ? THOOM_DIAG_MULTIPLIER : 1.0f) * 64.0f *
             thoom::game->delta;
  float new_x = this->x + dx;
  float new_y = this->y + dy;

  if (thoom::game->point_in_collider(new_x, new_y) ||
      thoom::game->ticks - this->spawned_ticks > 750) {
    thoom::game->delete_object = true;

    if (this->drop_item) {
      thoom::game->push_object(ITEM_CANNON_BALL DROPPED_OBJ,
                               DroppedItem::Options(this->x, this->y));
    }

    return;
  }

  this->x = new_x;
  this->y = new_y;

  this->dst_rect.x = float(this->x - 8);
  this->dst_rect.y = float(this->y - 8);

  thoom::game->push_sprite(this->sprite->tex_id, this->sprite->texture,
                           &this->sprite->frame, &this->dst_rect, 16);
}

void ThrownCannonBall::hitsource_notify() {
  this->did_hit = true;
  this->spawned_ticks = thoom::game->ticks - 500;  // 250ms until dropped
}
