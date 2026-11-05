#pragma once

#include "object.h"
#include "scene.h"
#include "sprite.h"

#define CREDITS_OBJ "credits"

class Credits : public thoom::Object {
 public:
  Credits();
  ~Credits() = default;

  void step() override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  std::string tex_id;
  SDL_Texture* texture;
  SDL_FRect src_rect, dst_rect;
};
