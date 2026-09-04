#pragma once

#include "game.h"
#include "object.h"

#define GAME_OVER_OBJ "game_over"

class GameOver : public thoom::Object {
 public:
  GameOver();
  ~GameOver();

  void step() override;

 private:
  SDL_Texture* ui;
  SDL_FRect src, dst;
};

class GameOverFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) { return new GameOver(); }
};
