#include "hud.h"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>

#include "bmp_texture.h"
#include "constants.h"
#include "controller.h"
#include "font.h"
#include "game.h"
#include "icon.h"
#include "net_agent.h"
#include "renderer.h"
#include "save_data.h"
#include "ui_box.h"

#define ITEM_NONE "item_none"

void Hud::init() {
  if (Hud::instance != nullptr) {
    std::cerr << "Hud::init error: hud already initialized" << std::endl;
    std::exit(1);
  }

  Hud::instance = new Hud();
}

void Hud::shutdown() {
  if (Hud::instance == nullptr) {
    return;
  }

  delete Hud::instance;
  Hud::instance = nullptr;
}

Hud::Hud() {
  thoom::Renderer* renderer = thoom::Renderer::instance;

  this->ui =
      renderer->create_texture(THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT,
                               SDL_PIXELFORMAT_RGBA32, SDL_SCALEMODE_LINEAR);
  this->overlay =
      renderer->create_texture(THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT,
                               SDL_PIXELFORMAT_RGBA32, SDL_SCALEMODE_LINEAR);

  if (this->ui == nullptr || this->overlay == nullptr) {
    std::cerr << "Hud::Hud error: " << SDL_GetError() << std::endl;
    exit(1);
  }

  renderer->clear(this->ui, thoom::kMask);
  renderer->clear(this->overlay, thoom::kMask);
}

Hud::~Hud() {
  SDL_DestroyTexture(this->ui);
  this->ui = nullptr;

  SDL_DestroyTexture(this->overlay);
  this->overlay = nullptr;
}

void Hud::render() {
  thoom::Renderer* renderer = thoom::Renderer::instance;

  thoom::Controller* controller = thoom::capture_controls
                                      ? &thoom::captured_controller
                                      : &thoom::local_controller;
  if (controller->is_hit(thoom::Button::MENU)) {
    controller->clear_all();
    this->display_controls_menu = !this->display_controls_menu;
    thoom::capture_controls = this->display_controls_menu;
  }

  if (thoom::game->current_map != this->current_map) {
    this->current_map = thoom::game->current_map;
    renderer->clear(this->ui, thoom::kMask);
    this->display_notification(thoom::game->map_title + ", " +
                               thoom::game->map_description);
  }

  bool force_render = false;

  if (this->display_controls_menu) {
    this->draw_controls_menu();
    return;
  } else if (this->displaying_controls_menu) {
    renderer->clear(this->overlay, thoom::kMask);
    this->displaying_controls_menu = false;
    force_render = true;
  }

  if (thoom::game->current_map == "maps/splash" ||
      thoom::game->current_map == "maps/init" ||
      thoom::game->current_map == "maps/dead" ||
      thoom::game->current_map == "maps/credits") {
    if (!force_render) {
      force_render = true;
      renderer->clear(this->overlay, thoom::kMask);
    }

    return;
  }

  this->draw_net_agent(force_render);
  this->draw_notification(force_render);
}

void Hud::display_notification(std::string text) {
  this->notification = text;
  this->notif_ticks = thoom::game->ticks;
  this->force_notif_rerender = true;
}

void Hud::draw_items(std::vector<Item> items, int sel_item, int health,
                     int max_health) {
  thoom::Renderer* renderer = thoom::Renderer::instance;

  SDL_FRect hud_rect = {HUD_MAIN_X, HUD_MAIN_Y, 18.0f, 48.0f};
  draw_ui_box(UI_BOX_MENU_CONT, this->ui, &hud_rect);
  renderer->draw_text(this->ui, thoom::game->fonts[SMALL_FONT],
                      "HP:", HUD_MAIN_X + 5, HUD_MAIN_Y + 4, 0, kBackground);

  SDL_FRect health_rect = {(float)(HUD_MAIN_X + 5), (float)(HUD_MAIN_Y + 11),
                           8.0f, 32.0f};
  int fill_px = ceil(30.0f * (float)health / (float)max_health) + 0.5f;
  SDL_FRect fill_rect = {(float)(HUD_MAIN_X + 6),
                         (float)(HUD_MAIN_Y + 12 + 30 - fill_px), 6.0f,
                         (float)fill_px};
  renderer->draw_rect(this->ui, &health_rect, thoom::Colour{52, 48, 48, 255},
                      SDL_BLENDMODE_NONE);
  renderer->draw_rect(this->ui, &fill_rect, thoom::Colour{240, 80, 64, 255},
                      SDL_BLENDMODE_NONE);

  std::string current_item = ITEM_NONE;
  int item_count = 1;

  if (sel_item >= 0 && sel_item < items.size()) {
    current_item = items[sel_item].item_id;
    item_count = items[sel_item].count;
  }

  SDL_FRect item_src_rect = {0.0f, 0.0f, 16.0f, 16.0f};
  SDL_Texture* item_texture;

  SDL_FRect clear_rect = this->item_list_rect;
  clear_rect.y -= 4.0f;
  clear_rect.h += 10.0f;
  renderer->draw_rect(this->ui, &clear_rect, thoom::kMask, SDL_BLENDMODE_NONE);

  this->item_list_rect.x = HUD_ITEMS_X;
  this->item_list_rect.y = HUD_ITEMS_Y;
  this->item_list_rect.w = float((items.size() + 1) * 18 + 8);
  this->item_list_rect.h = 24.0f;

  draw_ui_box(UI_BOX_CONTAINER, this->ui, &this->item_list_rect);

  int sel_offset = HUD_ITEMS_X + 5 + 18 * (sel_item + 1);
  SDL_FRect sel_outline_rect = {(float)(sel_offset - 1),
                                (float)(HUD_ITEMS_Y + 3), 18.0f, 18.0f};
  draw_ui_box(UI_BOX_OUT_SEL, this->ui, &sel_outline_rect);

  for (int i = -1; i < (int)items.size(); i++) {
    int x_offset = HUD_ITEMS_X + 5 + 18 * (i + 1);
    SDL_FRect item_rect = {(float)x_offset, (float)(HUD_ITEMS_Y + 4), 16.0f,
                           16.0f};

    std::string item_id = i < 0 ? ITEM_NONE : items[i].item_id;
    item_texture = thoom::load_bmp_texture("sprites/" + item_id + ".bmp");
    renderer->draw_texture(this->ui, item_texture, &item_src_rect, &item_rect);

    SDL_FRect hint_rect = {float(x_offset), HUD_ITEMS_Y + 15, 5.0f, 7.0f};
    renderer->draw_text(this->ui, thoom::game->fonts[SMALL_FONT],
                        std::to_string(i + 2), x_offset, HUD_ITEMS_Y + 15, 0,
                        kBackground);

    int this_item_count = i >= 0 ? items[i].count : 1;
    if (this_item_count > 1) {
      SDL_FRect item_count_icon = {(float)(x_offset + 8),
                                   (float)(HUD_ITEMS_Y - 4), 16.0f, 16.0f};
      draw_icon(i == sel_item ? ITEM_COUNT_ICON : ITEM_COUNT_ICON_SHD, this->ui,
                &item_count_icon);
      renderer->draw_text(this->ui, thoom::game->fonts[SMALL_FONT],
                          std::to_string(this_item_count),
                          this_item_count > 9 ? x_offset + 11 : x_offset + 13,
                          HUD_ITEMS_Y - 1, 0, kBackground);
    }
  }
}

void Hud::draw_controls_menu() {
  thoom::Renderer* renderer = thoom::Renderer::instance;
  bool should_render = !this->displaying_controls_menu;
  this->displaying_controls_menu = true;

  if (this->waiting_for_key) {
    if (thoom::captured_controller.last_input != SDLK_UNKNOWN) {
      SDL_Keycode new_key = thoom::captured_controller.last_input;
      thoom::Button sel_button = (thoom::Button)this->sel_control;

      if (thoom::ktobutton_map.find(new_key) != thoom::ktobutton_map.end()) {
        SDL_Keycode old_key = thoom::btokeycode_map[sel_button];
        thoom::Button existing_button = thoom::ktobutton_map[new_key];

        thoom::btokeycode_map[existing_button] = old_key;
        thoom::ktobutton_map[old_key] = existing_button;
      } else {
        SDL_Keycode old_key = thoom::btokeycode_map[sel_button];
        thoom::ktobutton_map.erase(old_key);
      }

      thoom::btokeycode_map[sel_button] = new_key;
      thoom::ktobutton_map[new_key] = sel_button;

      should_render = true;
      this->waiting_for_key = false;
    }
  } else {
    int y_dir = thoom::captured_controller.is_hit(thoom::Button::DOWN) -
                thoom::captured_controller.is_hit(thoom::Button::UP);
    int new_sel_control = THOOM_CLAMP(this->sel_control + y_dir, -3,
                                      (int)thoom::Button::DIGIT - 1);

    if (this->sel_control == -3 &&
        thoom::captured_controller.is_hit(thoom::Button::SELECT)) {
      thoom::game->map = "maps/init";
      thoom::save.clear();

      thoom::capture_controls = false;
      this->display_controls_menu = false;
      thoom::captured_controller.clear_all();

      return;
    }

    int x_dir = thoom::captured_controller.is_hit(thoom::Button::RIGHT) -
                thoom::captured_controller.is_hit(thoom::Button::LEFT);
    if (this->sel_control == -2 && x_dir != 0) {
      thoom::game->volume =
          THOOM_CLAMP(thoom::game->volume + (10 * x_dir), 0, 200);
      should_render = true;
    } else if (this->sel_control == -1 && x_dir != 0) {
      thoom::game->music_volume =
          THOOM_CLAMP(thoom::game->music_volume + (10 * x_dir), 0, 200);
      should_render = true;
    }

    if (this->sel_control != new_sel_control) {
      this->sel_control = new_sel_control;
      should_render = true;
    } else if (thoom::captured_controller.is_hit(thoom::Button::SELECT) &&
               this->sel_control >= 0) {
      thoom::captured_controller.last_input = SDLK_UNKNOWN;
      this->waiting_for_key = true;
      should_render = true;
    }
  }

  if (should_render) {
    renderer->clear(this->overlay,
                    thoom::Colour{thoom::game->bg_r, thoom::game->bg_g,
                                  thoom::game->bg_b, 128});

    SDL_FRect config_rect = {8.0f, 8.0f, 112.0f, 168.0f};
    draw_ui_box(UI_BOX_CONTAINER, this->overlay, &config_rect);

    renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT], "Game",
                        16, 16, 0, kBackground);

    renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT],
                        "Exit to Main Menu", 20, 28, 0, kBackground);
    if (this->sel_control != -3) {
      SDL_FRect option_rect = {20.0f, 28.0f, 96.0f, 8.0f};
      renderer->draw_rect(this->overlay, &option_rect,
                          thoom::Colour{24, 24, 24, 96}, SDL_BLENDMODE_BLEND);
    }

    renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT],
                        "Volume Settings", 16, 40, 0, kBackground);

    renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT], "Sounds",
                        20, 52, 0, kBackground);
    renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT],
                        std::to_string(thoom::game->volume), 52, 52, 0,
                        kBackground);
    if (this->sel_control != -2) {
      SDL_FRect sound_rect = {20.0f, 52.0f, 96.0f, 8.0f};
      renderer->draw_rect(this->overlay, &sound_rect,
                          thoom::Colour{24, 24, 24, 96}, SDL_BLENDMODE_BLEND);
      SDL_FRect volume_rect = {52.0f, 52.0f, 64.0f, 8.0f};
      renderer->draw_rect(this->overlay, &volume_rect,
                          thoom::Colour{24, 24, 24, 128}, SDL_BLENDMODE_BLEND);
    } else {
      SDL_FRect volume_rect = {52.0f, 52.0f, 64.0f, 8.0f};
      renderer->draw_rect(this->overlay, &volume_rect,
                          thoom::Colour{24, 24, 24, 96}, SDL_BLENDMODE_BLEND);
    }

    renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT], "Music",
                        20, 60, 0, kBackground);
    renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT],
                        std::to_string(thoom::game->music_volume), 52, 60, 0,
                        kBackground);
    if (this->sel_control != -1) {
      SDL_FRect music_rect = {20.0f, 60.0f, 96.0f, 8.0f};
      renderer->draw_rect(this->overlay, &music_rect,
                          thoom::Colour{24, 24, 24, 96}, SDL_BLENDMODE_BLEND);
      SDL_FRect volume_rect = {52.0f, 60.0f, 64.0f, 8.0f};
      renderer->draw_rect(this->overlay, &volume_rect,
                          thoom::Colour{24, 24, 24, 128}, SDL_BLENDMODE_BLEND);
    } else {
      SDL_FRect volume_rect = {52.0f, 60.0f, 64.0f, 8.0f};
      renderer->draw_rect(this->overlay, &volume_rect,
                          thoom::Colour{24, 24, 24, 96}, SDL_BLENDMODE_BLEND);
    }

    renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT],
                        "Controls", 16, 72, 0, kBackground);

    int start_y = 84;

    for (int i = 0; i < (int)thoom::Button::DIGIT; i++) {
      std::string control_name = thoom::btostring_map[(thoom::Button)i];
      std::string input_name =
          this->waiting_for_key && i == this->sel_control
              ? "<Press a Key>"
              : SDL_GetKeyName(thoom::btokeycode_map[(thoom::Button)i]);

      int y = start_y + (i * 8);
      renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT],
                          control_name, 20, y, 0, kBackground);
      renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT],
                          input_name, 52, y, 0, kBackground);

      int control_opacity = 128;
      int input_opacity = 192;
      if (i == this->sel_control) {
        if (this->waiting_for_key) {
          control_opacity = 96;
          input_opacity = 0;
        } else {
          control_opacity = 0;
          input_opacity = 96;
        }
      }

      SDL_FRect control_rect = {20.0f, (float)y, 28.0f, 8.0f};
      SDL_FRect input_rect = {52.0f, (float)y, 64.0f, 8.0f};
      renderer->draw_rect(this->overlay, &control_rect,
                          thoom::Colour{24, 24, 24, uint8_t(control_opacity)},
                          SDL_BLENDMODE_BLEND);
      renderer->draw_rect(this->overlay, &input_rect,
                          thoom::Colour{24, 24, 24, uint8_t(input_opacity)},
                          SDL_BLENDMODE_BLEND);
    }
  }
}

void Hud::draw_net_agent(bool force_render) {
  thoom::Renderer* renderer = thoom::Renderer::instance;

  if (!(thoom::game->create_objects && thoom::net_agent != nullptr)) return;

  std::string code = thoom::net_agent->get_connection_code();

  if (force_render || thoom::game->net_state != this->last_state ||
      code != this->last_code ||
      thoom::game->ticks - this->last_drawn_ticks > 1000) {
    if (this->netagent_rect.w > 0.0f) {
      renderer->draw_rect(this->overlay, &this->netagent_rect, thoom::kMask,
                          SDL_BLENDMODE_NONE);
    }

    switch (thoom::game->net_state) {
      case thoom::NetworkAgent::State::NO_CONNECTION:
        this->netagent_rect = SDL_FRect{8.0f, 8.0f, 88.0f, 16.0f};
        draw_ui_box(UI_BOX_MENU_CONT, this->overlay, &this->netagent_rect);
        draw_icon(NOT_CONNECTED_ICON, this->overlay, &this->neticon_rect);
        renderer->draw_text(this->overlay, thoom::game->fonts[DEFAULT_FONT],
                            "Not Connected", 28, 12, 0, kBackground);
        this->displaying_netagent = true;
        break;
      case thoom::NetworkAgent::State::WAITING_FOR_PEER:
        this->netagent_rect = SDL_FRect{8.0f, 8.0f, 80.0f, 16.0f};
        draw_ui_box(UI_BOX_MENU_CONT, this->overlay, &this->netagent_rect);
        draw_icon(WAITING_FOR_PEER_ICON, this->overlay, &this->neticon_rect);
        renderer->draw_text(this->overlay, thoom::game->fonts[SMALL_FONT],
                            "Code:", 28, 13, 0, kBackground);
        renderer->draw_text(this->overlay, thoom::game->fonts[CODE_FONT], code,
                            48, 12, 0, kBackground);
        this->displaying_netagent = true;
        break;
      default:
        renderer->clear(this->overlay, thoom::kMask);
        this->netagent_rect = SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};
        this->displaying_netagent = false;
        this->displaying_notification = false;
        break;
    }

    this->last_state = thoom::game->net_state;
    this->last_code = code;
    this->last_drawn_ticks = thoom::game->ticks;
  }
}

void Hud::draw_notification(bool force_render) {
  thoom::Renderer* renderer = thoom::Renderer::instance;

  if (this->notification.empty()) return;

  if (force_render || this->force_notif_rerender) {
    renderer->clear(this->overlay, thoom::kMask);
    this->displaying_notification = false;
    this->force_notif_rerender = false;

    if (this->displaying_netagent && !this->second_pass) {
      this->last_drawn_ticks = 0;
      this->second_pass = true;
      this->render();
      this->second_pass = false;
    }
  }

  if (!this->displaying_notification) {
    int text_w =
        thoom::game->fonts[DEFAULT_FONT]->text_width(this->notification);
    SDL_FRect notif_rect = SDL_FRect{8.0f, 8.0f, (float)(text_w + 12), 16.0f};
    int text_y = 12;

    if (this->displaying_netagent) {
      notif_rect.y = 26.0f;
      text_y = 30;
    }

    draw_ui_box(UI_BOX_MENU_CONT, this->overlay, &notif_rect);
    renderer->draw_text(this->overlay, thoom::game->fonts[DEFAULT_FONT],
                        this->notification, 14, text_y, 0, kBackground);

    this->displaying_notification = true;
  } else if (thoom::game->ticks - this->notif_ticks > 3000) {
    renderer->clear(this->overlay, thoom::kMask);
    this->notification.clear();
    this->displaying_notification = false;

    if (this->displaying_netagent && !this->second_pass) {
      this->last_drawn_ticks = 0;
      this->second_pass = true;
      this->render();
      this->second_pass = false;
    }
  }
}
