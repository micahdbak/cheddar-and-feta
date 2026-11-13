#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "init_host.h"
#include "init_join.h"
#include "object/object.h"

#define INIT_OBJ "init"

class Init : public thoom::Object {
 public:
  Init();
  ~Init();

  void step() override;
  void render() override;

  static thoom::Object* create_object(const std::string& options);

 private:
  enum Which { kChoose, kHost, kJoin };

  void draw_choice();

  SDL_Texture* ui = nullptr;
  SDL_FRect dst_rect = {0.0f, 0.0f, 320.0f, 240.0f};
  Which which = kChoose;
  int menu_sel = 0;
  bool redraw = true;
  InitHost init_host;
  InitJoin init_join;
};

bool init_show_peer(void* user);
