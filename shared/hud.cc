#include "hud.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <cstdlib>
#include <iostream>

#include "audio.h"
#include "constants.h"
#include "font.h"
#include "icon.h"
#include "input.h"
#include "peer.h"
#include "renderer.h"
#include "save_data.h"
#include "scene.h"
#include "texture.h"
#include "ui_box.h"
#include "window.h"

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
  this->ui = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA32,
      SDL_SCALEMODE_LINEAR);
  this->overlay = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA32,
      SDL_SCALEMODE_LINEAR);

  if (this->ui == nullptr || this->overlay == nullptr) {
    std::cerr << "Hud::Hud error: " << SDL_GetError() << std::endl;
    exit(1);
  }

  thoom::Renderer::instance->clear(this->ui, thoom::kMask);
  thoom::Renderer::instance->clear(this->overlay, thoom::kMask);
}

Hud::~Hud() {
  SDL_DestroyTexture(this->ui);
  this->ui = nullptr;

  SDL_DestroyTexture(this->overlay);
  this->overlay = nullptr;
}

void Hud::render() {
  thoom::Input* input = thoom::Input::instance;

  if (input->active->consume_hit(thoom::Button::MENU)) {
    this->display_controls_menu = !this->display_controls_menu;
    this->waiting_for_key = false;
    input->set_capture(this->display_controls_menu);
  }

  std::string current_map = thoom::Scene::instance->map != nullptr
                                ? thoom::Scene::instance->map->path
                                : "";

  if (current_map != this->current_map) {
    this->current_map = current_map;
    thoom::Renderer::instance->clear(this->ui, thoom::kMask);
    if (thoom::Scene::instance->map != nullptr) {
      this->display_notification(thoom::Scene::instance->map->title + ", " +
                                 thoom::Scene::instance->map->description);
    }
  }

  bool force_render = false;

  if (this->display_controls_menu) {
    this->draw_controls_menu();
    return;
  } else if (this->displaying_controls_menu) {
    thoom::Renderer::instance->clear(this->overlay, thoom::kMask);
    this->displaying_controls_menu = false;
    force_render = true;
  }

  if (current_map == "maps/splash" || current_map == "maps/init" ||
      current_map == "maps/dead" || current_map == "maps/credits") {
    if (!force_render) {
      force_render = true;
      thoom::Renderer::instance->clear(this->overlay, thoom::kMask);
    }

    return;
  }

  this->draw_peer(force_render);
  this->draw_notification(force_render);
}

void Hud::display_notification(std::string text) {
  this->notification = text;
  this->notif_ticks = thoom::Scene::instance->ticks;
  this->force_notif_rerender = true;
}

void Hud::draw_items(std::vector<Item> items, int sel_item, int health,
                     int max_health) {
  SDL_FRect hud_rect = {HUD_MAIN_X, HUD_MAIN_Y, 18.0f, 48.0f};
  draw_ui_box(UI_BOX_MENU_CONT, this->ui, &hud_rect);
  thoom::Renderer::instance->draw_text(this->ui, SMALL_FONT,
                                       "HP:", HUD_MAIN_X + 5, HUD_MAIN_Y + 4, 0,
                                       kBackground);

  SDL_FRect health_rect = {(float)(HUD_MAIN_X + 5), (float)(HUD_MAIN_Y + 11),
                           8.0f, 32.0f};
  int fill_px = ceil(30.0f * (float)health / (float)max_health) + 0.5f;
  SDL_FRect fill_rect = {(float)(HUD_MAIN_X + 6),
                         (float)(HUD_MAIN_Y + 12 + 30 - fill_px), 6.0f,
                         (float)fill_px};
  thoom::Renderer::instance->draw_rect(this->ui, &health_rect,
                                       thoom::Colour{52, 48, 48, 255},
                                       SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_rect(this->ui, &fill_rect,
                                       thoom::Colour{240, 80, 64, 255},
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
  thoom::Renderer::instance->draw_rect(this->ui, &clear_rect, thoom::kMask,
                                       SDL_BLENDMODE_NONE);

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
    item_texture =
        thoom::Texture::instance->load("sprites/" + item_id + ".bmp");
    thoom::Renderer::instance->draw_texture(this->ui, item_texture,
                                            &item_src_rect, &item_rect);

    SDL_FRect hint_rect = {float(x_offset), HUD_ITEMS_Y + 15, 5.0f, 7.0f};
    thoom::Renderer::instance->draw_text(this->ui, SMALL_FONT,
                                         std::to_string(i + 2), x_offset,
                                         HUD_ITEMS_Y + 15, 0, kBackground);

    int this_item_count = i >= 0 ? items[i].count : 1;
    if (this_item_count > 1) {
      SDL_FRect item_count_icon = {(float)(x_offset + 8),
                                   (float)(HUD_ITEMS_Y - 4), 16.0f, 16.0f};
      draw_icon(i == sel_item ? ITEM_COUNT_ICON : ITEM_COUNT_ICON_SHD, this->ui,
                &item_count_icon);
      thoom::Renderer::instance->draw_text(
          this->ui, SMALL_FONT, std::to_string(this_item_count),
          this_item_count > 9 ? x_offset + 11 : x_offset + 13, HUD_ITEMS_Y - 1,
          0, kBackground);
    }
  }
}

void Hud::draw_controls_menu() {
  thoom::Input* input = thoom::Input::instance;
  bool should_render = !this->displaying_controls_menu;
  this->displaying_controls_menu = true;

  SDL_Keycode keycode = input->captured.last_keycode;

  if (this->waiting_for_key) {
    if (keycode != SDLK_UNKNOWN) {
      input->bind_key((thoom::Button)this->sel_control, keycode);

      should_render = true;
      this->waiting_for_key = false;
    }
  } else {
    int y_dir = input->captured.consume_hit_dy();
    int new_sel_control = THOOM_CLAMP(this->sel_control + y_dir, -3,
                                      (int)thoom::Button::DIGIT - 1);

    if (this->sel_control == -3 &&
        input->captured.consume_hit(thoom::Button::SELECT)) {
      thoom::Scene::instance->request_map("maps/init");
      thoom::save.clear();

      this->display_controls_menu = false;
      this->waiting_for_key = false;
      input->set_capture(false);

      return;
    }

    int x_dir = input->captured.consume_hit_dx();
    if (this->sel_control == -2 && x_dir != 0) {
      thoom::Audio::instance->volume =
          THOOM_CLAMP(thoom::Audio::instance->volume + (10 * x_dir), 0, 200);
      should_render = true;
    } else if (this->sel_control == -1 && x_dir != 0) {
      thoom::Audio::instance->music_volume = THOOM_CLAMP(
          thoom::Audio::instance->music_volume + (10 * x_dir), 0, 200);
      should_render = true;
    }

    if (this->sel_control != new_sel_control) {
      this->sel_control = new_sel_control;
      should_render = true;
    } else if (input->captured.consume_hit(thoom::Button::SELECT) &&
               this->sel_control >= 0) {
      this->waiting_for_key = true;
      should_render = true;
    }
  }

  if (should_render) {
    thoom::Colour bg = thoom::kBlack;

    if (thoom::Scene::instance->map != nullptr) {
      bg = thoom::Scene::instance->map->background;
    }

    bg.a = 128;
    thoom::Renderer::instance->clear(this->overlay, bg);

    SDL_FRect config_rect = {8.0f, 8.0f, 112.0f, 168.0f};
    draw_ui_box(UI_BOX_CONTAINER, this->overlay, &config_rect);

    thoom::Renderer::instance->draw_text(this->overlay, SMALL_FONT, "Game", 16,
                                         16, 0, kBackground);

    thoom::Renderer::instance->draw_text(
        this->overlay, SMALL_FONT, "Exit to Main Menu", 20, 28, 0, kBackground);

    if (this->sel_control != -3) {
      SDL_FRect option_rect = {20.0f, 28.0f, 96.0f, 8.0f};
      thoom::Renderer::instance->draw_rect(this->overlay, &option_rect,
                                           thoom::Colour{24, 24, 24, 96},
                                           SDL_BLENDMODE_BLEND);
    }

    thoom::Renderer::instance->draw_text(
        this->overlay, SMALL_FONT, "Volume Settings", 16, 40, 0, kBackground);

    thoom::Renderer::instance->draw_text(this->overlay, SMALL_FONT, "Sounds",
                                         20, 52, 0, kBackground);
    thoom::Renderer::instance->draw_text(
        this->overlay, SMALL_FONT,
        std::to_string(thoom::Audio::instance->volume), 52, 52, 0, kBackground);

    if (this->sel_control != -2) {
      SDL_FRect sound_rect = {20.0f, 52.0f, 96.0f, 8.0f};
      thoom::Renderer::instance->draw_rect(this->overlay, &sound_rect,
                                           thoom::Colour{24, 24, 24, 96},
                                           SDL_BLENDMODE_BLEND);
      SDL_FRect volume_rect = {52.0f, 52.0f, 64.0f, 8.0f};
      thoom::Renderer::instance->draw_rect(this->overlay, &volume_rect,
                                           thoom::Colour{24, 24, 24, 128},
                                           SDL_BLENDMODE_BLEND);
    } else {
      SDL_FRect volume_rect = {52.0f, 52.0f, 64.0f, 8.0f};
      thoom::Renderer::instance->draw_rect(this->overlay, &volume_rect,
                                           thoom::Colour{24, 24, 24, 96},
                                           SDL_BLENDMODE_BLEND);
    }

    thoom::Renderer::instance->draw_text(this->overlay, SMALL_FONT, "Music", 20,
                                         60, 0, kBackground);
    thoom::Renderer::instance->draw_text(
        this->overlay, SMALL_FONT,
        std::to_string(thoom::Audio::instance->music_volume), 52, 60, 0,
        kBackground);

    if (this->sel_control != -1) {
      SDL_FRect music_rect = {20.0f, 60.0f, 96.0f, 8.0f};
      thoom::Renderer::instance->draw_rect(this->overlay, &music_rect,
                                           thoom::Colour{24, 24, 24, 96},
                                           SDL_BLENDMODE_BLEND);
      SDL_FRect volume_rect = {52.0f, 60.0f, 64.0f, 8.0f};
      thoom::Renderer::instance->draw_rect(this->overlay, &volume_rect,
                                           thoom::Colour{24, 24, 24, 128},
                                           SDL_BLENDMODE_BLEND);
    } else {
      SDL_FRect volume_rect = {52.0f, 60.0f, 64.0f, 8.0f};
      thoom::Renderer::instance->draw_rect(this->overlay, &volume_rect,
                                           thoom::Colour{24, 24, 24, 96},
                                           SDL_BLENDMODE_BLEND);
    }

    thoom::Renderer::instance->draw_text(this->overlay, SMALL_FONT, "Controls",
                                         16, 72, 0, kBackground);

    int start_y = 84;

    for (int i = 0; i < (int)thoom::Button::DIGIT; i++) {
      std::string control_name = input->bindings[i].name;
      std::string input_name = this->waiting_for_key && i == this->sel_control
                                   ? "<Press a Key>"
                                   : SDL_GetKeyName(input->bindings[i].keycode);

      int y = start_y + (i * 8);
      thoom::Renderer::instance->draw_text(this->overlay, SMALL_FONT,
                                           control_name, 20, y, 0, kBackground);
      thoom::Renderer::instance->draw_text(this->overlay, SMALL_FONT,
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
      thoom::Renderer::instance->draw_rect(
          this->overlay, &control_rect,
          thoom::Colour{24, 24, 24, uint8_t(control_opacity)},
          SDL_BLENDMODE_BLEND);
      thoom::Renderer::instance->draw_rect(
          this->overlay, &input_rect,
          thoom::Colour{24, 24, 24, uint8_t(input_opacity)},
          SDL_BLENDMODE_BLEND);
    }
  }
}

void Hud::draw_peer(bool force_render) {
  if (!(game_is_cheddar && thoom::Peer::instance != nullptr)) return;

  std::string code = thoom::Peer::instance->get_connection_code();
  uint64_t peer_state = thoom::Peer::instance->state.load();

  if (force_render || peer_state != this->last_peer_state ||
      code != this->last_code ||
      thoom::Scene::instance->ticks - this->last_drawn_ticks > 1000) {
    if (this->peer_rect.w > 0.0f) {
      thoom::Renderer::instance->draw_rect(this->overlay, &this->peer_rect,
                                           thoom::kMask, SDL_BLENDMODE_NONE);
    }

    switch (peer_state) {
      case thoom::Peer::kStateNoConnection:
        this->peer_rect = SDL_FRect{8.0f, 8.0f, 88.0f, 16.0f};
        draw_ui_box(UI_BOX_MENU_CONT, this->overlay, &this->peer_rect);
        draw_icon(NOT_CONNECTED_ICON, this->overlay, &this->peer_icon_rect);
        thoom::Renderer::instance->draw_text(this->overlay, DEFAULT_FONT,
                                             "Not Connected", 28, 12, 0,
                                             kBackground);
        this->displaying_peer = true;
        break;
      case thoom::Peer::kStateWaitingForPeer:
        this->peer_rect = SDL_FRect{8.0f, 8.0f, 80.0f, 16.0f};
        draw_ui_box(UI_BOX_MENU_CONT, this->overlay, &this->peer_rect);
        draw_icon(WAITING_FOR_PEER_ICON, this->overlay, &this->peer_icon_rect);
        thoom::Renderer::instance->draw_text(this->overlay, SMALL_FONT,
                                             "Code:", 28, 13, 0, kBackground);
        thoom::Renderer::instance->draw_text(this->overlay, CODE_FONT, code, 48,
                                             12, 0, kBackground);
        this->displaying_peer = true;
        break;
      default:
        thoom::Renderer::instance->clear(this->overlay, thoom::kMask);
        this->peer_rect = SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};
        this->displaying_peer = false;
        this->displaying_notification = false;
        break;
    }

    this->last_peer_state = peer_state;
    this->last_code = code;
    this->last_drawn_ticks = thoom::Scene::instance->ticks;
  }
}

void Hud::draw_notification(bool force_render) {
  if (this->notification.empty()) return;

  if (force_render || this->force_notif_rerender) {
    thoom::Renderer::instance->clear(this->overlay, thoom::kMask);
    this->displaying_notification = false;
    this->force_notif_rerender = false;

    if (this->displaying_peer && !this->second_pass) {
      this->last_drawn_ticks = 0;
      this->second_pass = true;
      this->render();
      this->second_pass = false;
    }
  }

  if (!this->displaying_notification) {
    int text_w = thoom::Renderer::instance->fonts[DEFAULT_FONT]->text_width(
        this->notification);
    SDL_FRect notif_rect = SDL_FRect{8.0f, 8.0f, (float)(text_w + 12), 16.0f};
    int text_y = 12;

    if (this->displaying_peer) {
      notif_rect.y = 26.0f;
      text_y = 30;
    }

    draw_ui_box(UI_BOX_MENU_CONT, this->overlay, &notif_rect);
    thoom::Renderer::instance->draw_text(this->overlay, DEFAULT_FONT,
                                         this->notification, 14, text_y, 0,
                                         kBackground);

    this->displaying_notification = true;
  } else if (thoom::Scene::instance->ticks - this->notif_ticks > 3000) {
    thoom::Renderer::instance->clear(this->overlay, thoom::kMask);
    this->notification.clear();
    this->displaying_notification = false;

    if (this->displaying_peer && !this->second_pass) {
      this->last_drawn_ticks = 0;
      this->second_pass = true;
      this->render();
      this->second_pass = false;
    }
  }
}
