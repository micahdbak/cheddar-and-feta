#include "molotov.h"

#include <cinttypes>
#include <cstdio>

#include "audio/audio.h"
#include "fire.h"
#include "hitbox.h"
#include "items/item.h"
#include "object/entity.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/log.h"
#include "thoom/macros.h"
#include "thoom/thoom.h"
#include "utils/utils.h"

ThrownMolotov::ThrownMolotov(const thoom::Entity::Context& context, float x,
                             float y, int x_dir, int y_dir, thoom_gid from_gid)
    : thoom::Entity(context), x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
  this->did_hit = false;
  this->sprite =
      new thoom::Sprite("sprites/item_molotov_thrown.bmp", 24, 24, 100);
  this->timer = thoom::Scene::instance->ticks;

  HitBox::Properties props = {1, 1000, 1000};
  thoom::Entity* instigator = thoom::Scene::instance->find_entity(from_gid);
  new HitBox(*this, instigator, -16, -16, 32, 32, props);
}

ThrownMolotov::~ThrownMolotov() { delete this->sprite; }

void ThrownMolotov::step() {
  float dx = float(this->x_dir) *
             (this->y_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * 128.0f *
             thoom::Scene::instance->delta;
  float dy = float(this->y_dir) *
             (this->x_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * 128.0f *
             thoom::Scene::instance->delta;
  float new_x = this->x + dx;
  float new_y = this->y + dy;

  if (this->did_hit || thoom::Scene::instance->ticks - this->timer > 1000 ||
      thoom::Scene::instance->map->collider->point_in_collider(new_x, new_y)) {
    for (int i = 0; i < 8; i++) {
      int fire_x_dir, fire_y_dir;
      thoom::dirs_from_direction(i, &fire_x_dir, &fire_y_dir);
      thoom::Scene::instance->create_entity(
          ITEM_FIRE USE_OBJ,
          UseItem::options(this->x, this->y, fire_x_dir, fire_y_dir, -1),
          {thoom::Entity::Ownership::kFixed, this->owner_pid});
    }

    thoom::Scene::instance->create_entity(
        ITEM_FIRE USE_OBJ, UseItem::options(this->x, this->y, 0, 0, -1),
        {thoom::Entity::Ownership::kFixed, this->owner_pid});

    this->deleted = true;

    thoom::Audio::instance->play("sfx/fire.wav", 1.0f, this->x, this->y, false);

    return;
  }

  this->x = new_x;
  this->y = new_y;

  this->sprite->update_frame();
}

void ThrownMolotov::render() {
  this->dst_rect.w = this->sprite->frame_w;
  this->dst_rect.h = this->sprite->frame_h;
  this->dst_rect.x = this->x - (float)(this->sprite->frame_w / 2);
  this->dst_rect.y = this->y - (float)(this->sprite->frame_h / 2);

  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 8);
}

thoom::Entity* DroppedMolotov::create_entity(
    const thoom::Entity::Context& context, const std::string& options) {
  int x, y;
  if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
    thoom_log_fatal("%s: bad options '%s'", __func__, options.c_str());
    return nullptr;
  }

  return new DroppedMolotov(context, float(x), float(y));
}

thoom::Entity* ThrownMolotov::create_entity(
    const thoom::Entity::Context& context, const std::string& options) {
  int x, y, x_dir = 0, y_dir = 0;
  thoom_gid from_gid;
  if (5 != sscanf(options.c_str(), "%d,%d,%d,%d,%" SCNu64, &x, &y, &x_dir,
                  &y_dir, &from_gid)) {
    thoom_log_fatal("%s: bad options '%s'", __func__, options.c_str());
    return nullptr;
  }

  return new ThrownMolotov(context, (float)x, (float)y, x_dir, y_dir, from_gid);
}
