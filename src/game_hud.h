#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#include "thoom/thoom.h"

class GameHud {
 public:
  struct Item {
    std::string item_id;
    int count;
  };

  static struct hud_def def(bool show_peer_enabled);
  static void draw_items(std::vector<Item> items, int sel_item, int health,
                         int max_health);

 private:
  static bool show_peer(void* user);
  static bool show_hud(void* user);
  static void exit_to_main_menu(void* user);

  inline static SDL_FRect item_list_rect = {0.0f, 0.0f, 0.0f, 0.0f};
};
