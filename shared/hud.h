#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>
#include <vector>

#include "peer.h"

#define UI_BOX_CONTAINER 0
#define UI_BOX_OUT 1
#define UI_BOX_OUT_SEL 2
#define UI_BOX_MENU_CONT 3
#define UI_BOX_MENU_SHD1 4
#define UI_BOX_MENU_SHD2 5
#define UI_BOX_CHAR_CONT 6
#define UI_BOX_CHAR_BOX 7
#define UI_BOX_CHAR_SEL 8
#define UI_BOX_CHAR_DISP 9
#define UI_BOX_MAP_TITLE 10

#define HUD_ITEMS_X 32
#define HUD_ITEMS_Y 204
#define HUD_MAIN_X 12
#define HUD_MAIN_Y 180

class Hud {
 public:
  struct Item {
    std::string item_id;
    int count;
  };

  static void init();
  static void shutdown();

  inline static Hud* instance = nullptr;

  Hud();
  ~Hud();

  void render();
  void display_notification(std::string text);
  void draw_items(std::vector<Item> items, int sel_item, int health,
                  int max_health);

  SDL_Texture* ui;
  SDL_Texture* overlay;

 private:
  void draw_controls_menu();
  void draw_peer(bool force_render);
  void draw_notification(bool force_render);

  std::string current_map;

  bool display_controls_menu = false;
  bool displaying_controls_menu = false;
  bool waiting_for_key = false;
  int sel_control = -3;

  uint64_t last_peer_state = thoom::Peer::kStateNoConnection;
  std::string last_code;
  Uint64 last_drawn_ticks = 0;
  bool displaying_peer = false;
  SDL_FRect peer_rect = {0.0f, 0.0f, 0.0f, 0.0f};
  SDL_FRect peer_icon_rect = {10.0f, 8.0f, 16.0f, 16.0f};

  std::string notification;
  Uint64 notif_ticks = 0;
  bool force_notif_rerender = false;
  bool displaying_notification = false;
  bool second_pass = false;

  SDL_FRect item_list_rect = {0.0f, 0.0f, 0.0f, 0.0f};
};
