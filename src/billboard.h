#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "object/entity.h"
#include "sprite/sprite.h"

#define BILLBOARD_OBJ "billboard"

class Billboard : public thoom::Entity {
 public:
  Billboard(const thoom::Entity::Context& context, int x, int y, int ts_x,
            int ts_y, int w, int h, int depth, std::string tilesheet);
  ~Billboard();

  void step() override {}
  void render() override;

  static void parse_options(const std::string& options, int* x, int* y,
                            int* ts_x, int* ts_y, int* w, int* h, int* depth,
                            std::string& tilesheet);
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  int x, y, depth;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
