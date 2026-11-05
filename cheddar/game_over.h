#pragma once

#include <string>

#include "object.h"
#include "scene.h"

#define GAME_OVER_OBJ "game_over"

class GameOver : public thoom::Object {
 public:
  GameOver();
  ~GameOver();

  void step() override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  SDL_Texture* ui;
  SDL_FRect src, dst;
};
