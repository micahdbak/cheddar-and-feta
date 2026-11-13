#include "cheese.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>

#include "items/item.h"
#include "mouse.h"
#include "object/entity.h"
#include "render_functions.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "texture/texture.h"
#include "thoom/thoom.h"

Cheese::Cheese(const thoom::Entity::Context& context, float x, float y,
               int amount)
    : thoom::Entity(context) {
  this->x = x;
  this->y = y;
  this->amount = amount;
  this->load_texture();
}

Cheese::~Cheese() {}

void Cheese::load_texture() {
  char args[256];
  snprintf(args, sizeof(args), "%d", this->amount);
  this->tex_id = RENDER_CHEESE + args;
  this->texture = thoom::Texture::instance->load(this->tex_id);

  this->depth_offset = this->texture->h - 2;
  this->dst_rect.w = float(this->texture->w);
  this->dst_rect.h = float(this->texture->h);
}

void Cheese::step() {
  Mouse* mouse = Mouse::closest_mouse(this->x, this->y, 8.0f, true);
  if (mouse != nullptr) {
    mouse->push_cheese(this->amount);
    this->deleted = true;
    return;
  }
}

void Cheese::render() {
  this->dst_rect.x = float(this->x - 9);
  this->dst_rect.y = float(this->y - this->depth_offset);
  thoom::Renderer::instance->push_sprite(this->tex_id, this->texture, NULL,
                                         &this->dst_rect, this->depth_offset);
}

void Cheese::drop_cheese(int x, int y, int min_amount, int max_amount) {
  static int consecutive_fails = 0;

  if (consecutive_fails < 4 && SDL_rand((1 + consecutive_fails) * 2) == 0) {
    consecutive_fails++;
    return;
  }

  consecutive_fails = 0;
  int amount =
      min_amount +
      ((max_amount - min_amount) > 0 ? SDL_rand(max_amount - min_amount) : 0);
  char cheese_opt[256];
  thoom::Scene::instance->create_entity(
      ITEM_CHEESE DROPPED_OBJ, Cheese::options(x, y, amount),
      {thoom::Entity::Ownership::kRoundRobin, THOOM_PID_INVALID});
}

thoom::Entity* Cheese::create_entity(const thoom::Entity::Context& context,
                                     const std::string& options) {
  int x, y, amount;
  int fields = sscanf(options.c_str(), "%d,%d,%d", &x, &y, &amount);

  if (fields == 2) {
    amount = 1;
  } else if (fields != 3) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new Cheese(context, float(x), float(y), amount);
}
