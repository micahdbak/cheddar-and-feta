#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

#include "object.h"
#include "peer/peer.h"
#include "scene/scene.h"

class Launch : public thoom::Object {
 public:
  Launch();
  ~Launch();

  void step() override;

  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  bool choose_mode = true;
  bool render = true, waiting_for_peer = false;
  int menu_sel = 0;
  int sel_c = 0;
  std::string code;
  SDL_Texture* ui = nullptr;
  SDL_FRect dst_rect = {0.0f, 0.0f, 320.0f, 240.0f};
  uint64_t last_peer_state = thoom::Peer::kStateNoConnection;
  Uint64 waiting_ticks = 0;
};

bool launch_show_peer(void* user);
