#include "toothpick.h"

#include <cinttypes>
#include <cstdio>
#include <string>

#include "hitbox.h"
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

ThrownToothpick::ThrownToothpick(const thoom::Entity::Context& context, float x,
                                 float y, int x_dir, int y_dir,
                                 thoom_gid from_gid)
    : thoom::Entity(context), x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
  thoom::Entity* obj = thoom::Scene::instance->find_entity(from_gid);
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
  new HitBox(*this, obj, -8, -8, 16, 16, props);
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
      thoom::Scene::instance->create_entity(
          ITEM_TOOTHPICK DROPPED_OBJ, DroppedItem::options(this->x, this->y),
          {thoom::Entity::Ownership::kRoundRobin, THOOM_PID_INVALID});
    }

    return;
  }

  this->x = new_x;
  this->y = new_y;
}

void ThrownToothpick::render() {
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

thoom::Entity* DroppedToothpick::create_entity(
    const thoom::Entity::Context& context, const std::string& options) {
  int x, y;
  if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
    thoom_log_fatal("%s: bad options '%s'", __func__, options.c_str());
    return nullptr;
  }

  return new DroppedToothpick(context, float(x), float(y));
}

thoom::Entity* ThrownToothpick::create_entity(
    const thoom::Entity::Context& context, const std::string& options) {
  int x, y, x_dir, y_dir;
  thoom_gid from_gid;
  if (5 != sscanf(options.c_str(), "%d,%d,%d,%d,%" SCNu64, &x, &y, &x_dir,
                  &y_dir, &from_gid)) {
    thoom_log_fatal("%s: bad options '%s'", __func__, options.c_str());
    return nullptr;
  }

  return new ThrownToothpick(context, (float)x, (float)y, x_dir, y_dir,
                             from_gid);
}
