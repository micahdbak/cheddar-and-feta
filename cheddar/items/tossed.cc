#include "tossed.h"

#include "game.h"
#include "item.h"
#include "utils.h"

TossedItem::TossedItem(float x, float y, int x_dir, int y_dir, bool from_feta,
                       std::string item_id)
    : x(x),
      y(y),
      x_dir(x_dir),
      y_dir(y_dir),
      from_feta(from_feta),
      item_id(item_id) {
  std::string item_sprite_path = "sprites/" + item_id + ".bmp";
  this->sprite = new thoom::Sprite(item_sprite_path.c_str(), 16, 16, 0);

  this->dst_rect.w = 16.0f;
  this->dst_rect.h = 16.0f;

  this->spawned_ticks = thoom::game->ticks;
}

TossedItem::~TossedItem() { delete this->sprite; }

void TossedItem::step() {
  float dx = float(this->x_dir) *
             (this->y_dir != 0 ? THOOM_DIAG_MULTIPLIER : 1.0f) * 128.0f *
             thoom::game->delta;
  float dy = float(this->y_dir) *
             (this->x_dir != 0 ? THOOM_DIAG_MULTIPLIER : 1.0f) * 128.0f *
             thoom::game->delta;
  float new_x = this->x + dx;
  float new_y = this->y + dy;

  float mouse_distance = 32.0f;
  if (from_feta) {
    mouse_distance =
        THOOM_DISTANCE_BETWEEN_POINTS(new_x, new_y, cheddar->x, cheddar->y);
  } else {
    mouse_distance =
        THOOM_DISTANCE_BETWEEN_POINTS(new_x, new_y, feta->x, feta->y);
  }

  if (thoom::game->point_in_collider(new_x, new_y) || mouse_distance < 8.0f ||
      thoom::game->ticks - this->spawned_ticks > 500) {
    thoom::game->delete_object = true;

    std::string dropped_obj_id = this->item_id + DROPPED_OBJ;
    thoom::game->push_object(dropped_obj_id,
                             DroppedItem::Options(this->x, this->y));

    return;
  }

  this->x = new_x;
  this->y = new_y;

  this->dst_rect.x = this->x - 8.0f;
  this->dst_rect.y = this->y - 8.0f;

  thoom::game->push_sprite(this->sprite->tex_id, this->sprite->texture,
                           &this->sprite->frame, &this->dst_rect, 16);
}
