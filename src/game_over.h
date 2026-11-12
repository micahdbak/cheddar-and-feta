#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "object/object.h"

#define GAME_OVER_OBJ "game_over"

class GameOver : public thoom::Object {
 public:
  GameOver();
  ~GameOver();

  void step() override;
  void render() override;
  static thoom::Object* create_object(const std::string& options);

 private:
  SDL_Texture* ui;
  SDL_FRect src, dst;
};
