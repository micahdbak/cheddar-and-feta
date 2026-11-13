#include "tossed.h"

#include <cstdio>
#include <string>
#include <vector>

#include "item.h"
#include "mouse.h"
#include "object/entity.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/log.h"
#include "thoom/macros.h"
#include "thoom/thoom.h"
#include "utils/utils.h"

TossedItem::TossedItem(const thoom::Entity::Context& context, float x, float y,
                       int x_dir, int y_dir, bool from_feta,
                       std::string item_id)
    : thoom::Entity(context),
      x(x),
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
  const std::vector<thoom::Entity*> entities =
      thoom::Scene::instance->find_entities(
          thoom::Entity::AuthorityScope::kAll);
  for (thoom::Entity* entity : entities) {
    Mouse* mouse = dynamic_cast<Mouse*>(entity);
    if (mouse == nullptr || mouse->is_feta == this->from_feta) {
      continue;
    }

    const float distance =
        thoom::distance_between_points(new_x, new_y, mouse->x, mouse->y);
    mouse_distance = THOOM_MIN(mouse_distance, distance);
  }

  if (thoom::Scene::instance->map->collider->point_in_collider(new_x, new_y) ||
      mouse_distance < 8.0f ||
      thoom::Scene::instance->ticks - this->spawned_ticks > 500) {
    this->deleted = true;

    std::string dropped_obj_id = this->item_id + DROPPED_OBJ;
    thoom::Scene::instance->create_entity(
        dropped_obj_id, DroppedItem::options(this->x, this->y),
        {thoom::Entity::Ownership::kRoundRobin, THOOM_PID_INVALID});

    return;
  }

  this->x = new_x;
  this->y = new_y;
}

void TossedItem::render() {
  this->dst_rect.x = this->x - 8.0f;
  this->dst_rect.y = this->y - 8.0f;

  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 16);
}

thoom::Entity* TossedItem::create_entity(const thoom::Entity::Context& context,
                                         const std::string& options) {
  int x, y, x_dir, y_dir, from_feta;
  if (5 != sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir,
                  &from_feta)) {
    thoom_log_fatal("%s: bad options '%s'", __func__, options.c_str());
    return nullptr;
  }

  const char* arr = options.c_str();
  while (*arr != ' ' && *arr != '\0') arr++;

  if (*arr == ' ') arr++;

  if (*arr == '\0') {
    thoom_log_warn("TossedItem: missing item id in options '%s'",
                   options.c_str());
    return nullptr;
  }

  return new TossedItem(context, x, y, x_dir, y_dir, from_feta == 1,
                        std::string(arr));
}
