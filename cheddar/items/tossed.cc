#include "tossed.h"

#include "item.h"
#include "renderer.h"
#include "scene.h"
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

  this->spawned_ticks = thoom::Scene::instance->ticks;
}

TossedItem::~TossedItem() { delete this->sprite; }

void TossedItem::step() {
  float dx = float(this->x_dir) *
             (this->y_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * 128.0f *
             thoom::Scene::instance->delta;
  float dy = float(this->y_dir) *
             (this->x_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * 128.0f *
             thoom::Scene::instance->delta;
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

  if (thoom::Scene::instance->map->collider->point_in_collider(new_x, new_y) ||
      mouse_distance < 8.0f ||
      thoom::Scene::instance->ticks - this->spawned_ticks > 500) {
    this->deleted = true;

    std::string dropped_obj_id = this->item_id + DROPPED_OBJ;
    thoom::Scene::instance->create_object(
        dropped_obj_id, DroppedItem::options(this->x, this->y));

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

thoom::Object* TossedItem::create_object(thoom::Scene* scene,
                                         const std::string& options) {
  int x, y, x_dir, y_dir, from_feta;
  if (5 != sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir,
                  &from_feta)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  const char* arr = options.c_str();
  while (*arr != ' ' && *arr != '\0') arr++;

  if (*arr == ' ') arr++;

  if (*arr == '\0') return nullptr;

  return new TossedItem(x, y, x_dir, y_dir, from_feta == 1, std::string(arr));
}
