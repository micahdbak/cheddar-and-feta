#include "game_hud.h"

#include <cmath>
#include <string>
#include <vector>

#include "colour.h"
#include "constants.h"
#include "hud/hud.h"
#include "icon.h"
#include "renderer/font.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "texture/texture.h"
#include "thoom/thoom.h"

#define ITEM_NONE "item_none"

#define HUD_ITEMS_X 32
#define HUD_ITEMS_Y 204
#define HUD_MAIN_X 12
#define HUD_MAIN_Y 180

struct hud_def GameHud::def(bool show_peer_enabled) {
  struct hud_def def = {};
  def.show_peer = show_peer_enabled ? GameHud::show_peer : nullptr;
  def.show_hud = GameHud::show_hud;
  def.exit_to_main_menu = GameHud::exit_to_main_menu;
  return def;
}

void GameHud::draw_items(std::vector<Item> items, int sel_item, int health,
                         int max_health) {
  SDL_Texture* ui = thoom::Hud::instance->ui;

  SDL_FRect hud_rect = {HUD_MAIN_X, HUD_MAIN_Y, 18.0f, 48.0f};
  thoom::Renderer::instance->draw_ui_box(UI_BOX_MENU_CONT, ui, &hud_rect);
  thoom::Renderer::instance->draw_text(ui, SMALL_FONT, "HP:", HUD_MAIN_X + 5,
                                       HUD_MAIN_Y + 4, 0, kBackground);

  SDL_FRect health_rect = {(float)(HUD_MAIN_X + 5), (float)(HUD_MAIN_Y + 11),
                           8.0f, 32.0f};
  int fill_px = ceil(30.0f * (float)health / (float)max_health) + 0.5f;
  SDL_FRect fill_rect = {(float)(HUD_MAIN_X + 6),
                         (float)(HUD_MAIN_Y + 12 + 30 - fill_px), 6.0f,
                         (float)fill_px};
  thoom::Renderer::instance->draw_rect(
      ui, &health_rect, thoom::Colour{52, 48, 48, 255}, SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_rect(
      ui, &fill_rect, thoom::Colour{240, 80, 64, 255}, SDL_BLENDMODE_NONE);

  SDL_FRect item_src_rect = {0.0f, 0.0f, 16.0f, 16.0f};

  SDL_FRect clear_rect = item_list_rect;
  clear_rect.y -= 4.0f;
  clear_rect.h += 10.0f;
  thoom::Renderer::instance->draw_rect(ui, &clear_rect, thoom::kMask,
                                       SDL_BLENDMODE_NONE);

  item_list_rect.x = HUD_ITEMS_X;
  item_list_rect.y = HUD_ITEMS_Y;
  item_list_rect.w = float((items.size() + 1) * 18 + 8);
  item_list_rect.h = 24.0f;

  thoom::Renderer::instance->draw_ui_box(UI_BOX_CONTAINER, ui, &item_list_rect);

  int sel_offset = HUD_ITEMS_X + 5 + 18 * (sel_item + 1);
  SDL_FRect sel_outline_rect = {(float)(sel_offset - 1),
                                (float)(HUD_ITEMS_Y + 3), 18.0f, 18.0f};
  thoom::Renderer::instance->draw_ui_box(UI_BOX_OUT_SEL, ui, &sel_outline_rect);

  for (int i = -1; i < (int)items.size(); i++) {
    int x_offset = HUD_ITEMS_X + 5 + 18 * (i + 1);
    SDL_FRect item_rect = {(float)x_offset, (float)(HUD_ITEMS_Y + 4), 16.0f,
                           16.0f};

    std::string item_id = i < 0 ? ITEM_NONE : items[i].item_id;
    SDL_Texture* item_texture =
        thoom::Texture::instance->load("sprites/" + item_id + ".bmp");
    thoom::Renderer::instance->draw_texture(ui, item_texture, &item_src_rect,
                                            &item_rect);

    thoom::Renderer::instance->draw_text(ui, SMALL_FONT, std::to_string(i + 2),
                                         x_offset, HUD_ITEMS_Y + 15, 0,
                                         kBackground);

    int this_item_count = i >= 0 ? items[i].count : 1;
    if (this_item_count > 1) {
      SDL_FRect item_count_icon = {(float)(x_offset + 8),
                                   (float)(HUD_ITEMS_Y - 4), 16.0f, 16.0f};
      draw_icon(i == sel_item ? ITEM_COUNT_ICON : ITEM_COUNT_ICON_SHD, ui,
                &item_count_icon);
      thoom::Renderer::instance->draw_text(
          ui, SMALL_FONT, std::to_string(this_item_count),
          this_item_count > 9 ? x_offset + 11 : x_offset + 13, HUD_ITEMS_Y - 1,
          0, kBackground);
    }
  }
}

bool GameHud::show_peer(void* user) { return true; }

bool GameHud::show_hud(void* user) {
  if (thoom::Scene::instance == nullptr ||
      thoom::Scene::instance->map == nullptr) {
    return true;
  }

  const std::string& path = thoom::Scene::instance->map->path;
  return path != "maps/splash" && path != "maps/init" && path != "maps/dead" &&
         path != "maps/credits";
}

void GameHud::exit_to_main_menu(void* user) {
  thoom::Scene::instance->request_map("maps/init", true);
  thoom::save.clear();
}
