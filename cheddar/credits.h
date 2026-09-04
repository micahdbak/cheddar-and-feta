#pragma once

#include "object.h"
#include "sprite.h"

#define CREDITS_OBJ "credits"

class Credits : public thoom::Object {
 public:
  Credits();
  ~Credits() = default;

  void step() override;

 private:
  std::string tex_id;
  SDL_Texture* texture;
  SDL_FRect src_rect, dst_rect;
};

class CreditsFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& _) { return new Credits(); };
};
