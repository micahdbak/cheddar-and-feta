#pragma once

#include <cstdio>
#include <iostream>

#include "object.h"
#include "scene.h"
#include "sprite.h"

#define BILLBOARD_OBJ "billboard"

class Billboard : public thoom::Object {
 public:
  Billboard(int x, int y, int ts_x, int ts_y, int w, int h, int depth,
            std::string tilesheet);
  ~Billboard();

  void step() override;

  static void parse_options(const std::string& options, int* x, int* y,
                            int* ts_x, int* ts_y, int* w, int* h, int* depth,
                            std::string& tilesheet);
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int x, y, depth;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
