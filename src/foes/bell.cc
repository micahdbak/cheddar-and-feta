#include "bell.h"

#include <cstdio>
#include <cstdlib>

#include "audio/audio.h"
#include "hurtbox.h"
#include "object/entity.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "spawner.h"
#include "sprite/sprite.h"

Bell::Bell(const thoom::Entity::Context& context, int x, int y, int spawner_id)
    : thoom::Entity(context) {
  this->x = x;
  this->y = y;
  this->spawner_id = spawner_id;
  this->sprite = new thoom::Sprite("sprites/bell.bmp", 32, 32, 125);
  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;

  new HurtBox(*this, -8, -8, 16, 16);
}

Bell::~Bell() { delete this->sprite; }

void Bell::attack(int damage) {
  FoeSpawner* spawner = spawners[this->spawner_id];

  if (spawner != nullptr && !spawner->empty && !this->triggered) {
    spawners[this->spawner_id]->trigger();
  }

  // don't ring while ringing
  if (thoom::Scene::instance->ticks - this->timer < 2250) {
    return;
  }

  this->triggered = true;
  this->timer = thoom::Scene::instance->ticks;
  thoom::Audio::instance->play("sfx/bell.wav", 1.0f, this->x, this->y, false);
}

void Bell::step() {
  if (this->triggered && thoom::Scene::instance->ticks - this->timer < 2250) {
    this->sprite->update_frame();
  } else {
    this->sprite->set_frame(0);
  }
}

void Bell::render() {
  this->dst_rect.x = this->x - 16.0f;
  this->dst_rect.y = this->y - 16.0f;
  thoom::Renderer::instance->push_sprite(
      "sprites/bell.bmp", this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 16);
}

thoom::Entity* Bell::create_entity(const thoom::Entity::Context& context,
                                   const std::string& options) {
  int x = 0, y = 0, spawner_id = 0;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new Bell(context, x, y, spawner_id);
}
