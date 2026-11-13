#pragma once

#include <SDL3/SDL.h>

#include <cstdio>
#include <string>

#include "object/entity.h"

#define ITEM_CHEESE "item_cheese"

// dropped cheese
class Cheese : public thoom::Entity {
 public:
  static std::string options(int x, int y, int amount) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d", x, y, amount);
    return std::string(buff);
  }

  Cheese(const thoom::Entity::Context& context, float x, float y, int amount);
  ~Cheese();

  void step() override;
  void render() override;

  static void drop_cheese(int x, int y, int min_amount, int max_amount);
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  void load_texture();

  std::string tex_id;
  SDL_Texture* texture = nullptr;
  SDL_FRect dst_rect;
  float x, y;
  int amount, depth_offset;
};
