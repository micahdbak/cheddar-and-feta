#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "object/object.h"

#define CREDITS_OBJ "credits"

class Credits : public thoom::Object {
 public:
  Credits();
  ~Credits() = default;

  void step() override;
  void render() override;
  static thoom::Object* create_object(const std::string& options);

 private:
  std::string tex_id;
  SDL_Texture* texture;
  SDL_FRect src_rect, dst_rect;
};
