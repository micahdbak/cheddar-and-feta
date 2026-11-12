#include "fire.h"

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "hitbox.h"
#include "object/entity.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/macros.h"

static int fire_counter = 0;

Fire::Fire(const thoom::Entity::Context& context, float x, float y, int x_dir,
           int y_dir)
    : thoom::Entity(context), x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
  this->sprite = new thoom::Sprite("sprites/fire.bmp", 16, 16, 100);
  this->dst_rect.w = this->sprite->frame_w;
  this->dst_rect.h = this->sprite->frame_h;
  this->timer = thoom::Scene::instance->ticks;

  fire_counter++;
  this->sprite->set_animation(fire_counter % 2);
  this->sprite->set_frame(SDL_rand(4));

  this->mov_speed = SDL_randf() * 64.0f + 120.0f;
  this->throw_time = 100;

  if (!x_dir && !y_dir) {
    this->state = Fire::State::STATIONARY;
  } else {
    this->throw_time = THOOM_MAX(THOOM_ABS(x_dir), THOOM_ABS(y_dir)) * 100;
  }

  // deals 3 damage over 3 seconds
  new HitBox(*this, nullptr, -8, -8, 16, 16,
             HitBox::Properties{.damage = 1,
                                .cooldown_ms = 1000,
                                .delete_after_ms = 4000,
                                .shared_cooldowns = true,
                                .shared_id = HB_SHARED_FIRE});
}

Fire::~Fire() { delete this->sprite; }

void Fire::step() {
  if (this->state == Fire::State::MOVING) {
    float dx = float(this->x_dir) *
               (this->y_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * mov_speed *
               thoom::Scene::instance->delta;
    float dy = float(this->y_dir) *
               (this->x_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) * mov_speed *
               thoom::Scene::instance->delta;
    float new_x = this->x + dx;
    float new_y = this->y + dy;

    if (!thoom::Scene::instance->map->collider->point_in_collider(new_x,
                                                                  this->y)) {
      this->x = new_x;
    }

    if (!thoom::Scene::instance->map->collider->point_in_collider(this->x,
                                                                  new_y)) {
      this->y = new_y;
    }

    if (thoom::Scene::instance->ticks - this->timer > this->throw_time) {
      this->state = Fire::State::STATIONARY;
    }
  }

  if (thoom::Scene::instance->ticks - this->timer > 3000) {
    this->deleted = true;
    return;
  } else if (thoom::Scene::instance->ticks - this->timer > 2875) {
    this->sprite->set_animation(3);
  } else if (thoom::Scene::instance->ticks - this->timer > 2750) {
    this->sprite->set_animation(2);
    this->sprite->interval_ms = 1000;
  }

  this->sprite->update_frame();
}

void Fire::render() {
  this->dst_rect.x = this->x - (float)(this->sprite->frame_w / 2);
  this->dst_rect.y = this->y - (float)(this->sprite->frame_h / 2);

  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 12);
}

thoom::Entity* Fire::create_entity(const thoom::Entity::Context& context,
                                   const std::string& options) {
  int x, y, x_dir = 0, y_dir = 0;
  if (4 != sscanf(options.c_str(), "%d,%d,%d,%d,%*" SCNu64, &x, &y, &x_dir,
                  &y_dir)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new Fire(context, (float)x, (float)y, x_dir, y_dir);
}
