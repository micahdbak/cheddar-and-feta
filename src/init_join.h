#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

class InitJoin {
 public:
  void start(SDL_Texture* ui);
  void step();
  void render();

 private:
  SDL_Texture* ui = nullptr;
  bool redraw = true, waiting_for_peer = false;
  int sel_c = 0;
  std::string code;
  uint64_t peer_state = 0, last_peer_state = 0;
  Uint64 waiting_ticks = 0;
};
