#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#define INIT_HOST_OBJ "init_host"

class InitHost {
 public:
  void start(SDL_Texture* ui);
  void step();
  void render();

 private:
  SDL_Texture* ui = nullptr;
  std::vector<std::string> summaries;
  int sel_save = 0, save_start = 0, save_end = 0;
  bool redraw = true;
};
