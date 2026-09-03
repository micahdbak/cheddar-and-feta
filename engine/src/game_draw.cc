#include <SDL3/SDL.h>

#include <iostream>

#include "SDL3/SDL_blendmode.h"
#include "bmp_texture.h"
#include "controller.h"
#include "font.h"
#include "game.h"
#include "map.h"
#include "save_data.h"

void Game::draw_rect(SDL_Texture* texture, SDL_FRect* rect, Uint8 r, Uint8 g,
                     Uint8 b, Uint8 a, SDL_BlendMode blend_mode) {
  SDL_SetRenderTarget(renderer, texture);

  SDL_SetRenderDrawColor(renderer, r, g, b, a);
  SDL_SetRenderDrawBlendMode(renderer, blend_mode);
  SDL_RenderFillRect(renderer, rect);

  SDL_SetRenderTarget(renderer, this->screen);
}

void Game::draw_outline(SDL_Texture* texture, SDL_FRect* rect, Uint8 r, Uint8 g,
                        Uint8 b, Uint8 a, SDL_BlendMode blend_mode) {
  SDL_SetRenderTarget(renderer, texture);

  SDL_SetRenderDrawColor(renderer, r, g, b, a);
  SDL_SetRenderDrawBlendMode(renderer, blend_mode);
  SDL_RenderRect(renderer, rect);

  SDL_SetRenderTarget(renderer, this->screen);
}

#define UI_BOX_SIZE 4.0f

void Game::draw_ui_box(SDL_Texture* texture, int type, SDL_FRect* rect) {
  SDL_SetRenderTarget(renderer, texture);

  SDL_FRect _rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
  if (rect == NULL) {
    rect = &_rect;
  }

  int x_shift = type * ((int)UI_BOX_SIZE * 3);
  SDL_FRect src_rect, dst_rect;

  // top-left corner
  src_rect = {float(x_shift), 0.0f, UI_BOX_SIZE, UI_BOX_SIZE};
  dst_rect = {rect->x, rect->y, UI_BOX_SIZE, UI_BOX_SIZE};
  SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

  // top-right corner
  src_rect = {float(x_shift + 2 * UI_BOX_SIZE), 0.0f, UI_BOX_SIZE, UI_BOX_SIZE};
  dst_rect = {rect->x + rect->w - UI_BOX_SIZE, rect->y, UI_BOX_SIZE,
              UI_BOX_SIZE};
  SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

  // bottom-right corner
  src_rect = {float(x_shift + 2 * UI_BOX_SIZE), UI_BOX_SIZE * 2.0f, UI_BOX_SIZE,
              UI_BOX_SIZE};
  dst_rect = {rect->x + rect->w - UI_BOX_SIZE, rect->y + rect->h - UI_BOX_SIZE,
              UI_BOX_SIZE, UI_BOX_SIZE};
  SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

  // bottom-left corner
  src_rect = {float(x_shift), UI_BOX_SIZE * 2.0f, UI_BOX_SIZE, UI_BOX_SIZE};
  dst_rect = {rect->x, rect->y + rect->h - UI_BOX_SIZE, UI_BOX_SIZE,
              UI_BOX_SIZE};
  SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

  // top edge
  src_rect = {float(x_shift + UI_BOX_SIZE), 0.0f, UI_BOX_SIZE, UI_BOX_SIZE};
  dst_rect = {rect->x + UI_BOX_SIZE, rect->y, rect->w - (UI_BOX_SIZE * 2.0f),
              UI_BOX_SIZE};
  SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

  // right edge
  src_rect = {float(x_shift + 2 * UI_BOX_SIZE), UI_BOX_SIZE, UI_BOX_SIZE,
              UI_BOX_SIZE};
  dst_rect = {rect->x + rect->w - UI_BOX_SIZE, rect->y + UI_BOX_SIZE,
              UI_BOX_SIZE, rect->h - (UI_BOX_SIZE * 2.0f)};
  SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

  // bottom edge
  src_rect = {float(x_shift + UI_BOX_SIZE), UI_BOX_SIZE * 2.0f, UI_BOX_SIZE,
              UI_BOX_SIZE};
  dst_rect = {rect->x + UI_BOX_SIZE, rect->y + rect->h - UI_BOX_SIZE,
              rect->w - (UI_BOX_SIZE * 2.0f), UI_BOX_SIZE};
  SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

  // left edge
  src_rect = {float(x_shift), UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE};
  dst_rect = {rect->x, rect->y + UI_BOX_SIZE, UI_BOX_SIZE,
              rect->h - (UI_BOX_SIZE * 2.0f)};
  SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

  // middle
  src_rect = {float(x_shift + UI_BOX_SIZE), UI_BOX_SIZE, UI_BOX_SIZE,
              UI_BOX_SIZE};
  dst_rect = {rect->x + UI_BOX_SIZE, rect->y + UI_BOX_SIZE,
              rect->w - (UI_BOX_SIZE * 2.0f), rect->h - (UI_BOX_SIZE * 2.0f)};
  SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

  SDL_SetRenderTarget(renderer, this->screen);
}

void Game::draw_text(SDL_Texture* texture, std::string str, int font, int x,
                     int y, int w) {
  if (font < 0 || font >= NUM_FONTS) {
    std::cerr << "Game::draw_text error: '" << font << "' font does not exist"
              << std::endl;
    exit(1);
  }

  SDL_SetRenderTarget(renderer, texture);

  Font* _font = this->fonts[font];
  int _x = x, _y = y, line_height = int(_font->src_rect[0].h) + 1;
  int running_width = 0;
  SDL_FRect dst_rect;
  SDL_SetRenderDrawColor(renderer, 24, 24, 24, 255);

  for (int i = 0; i < str.size(); i++) {
    char c = str[i];
    if (c >= ' ' && c <= '~') {
      SDL_FRect* src_rect = _font->src_rect + (c - ' ');

      if (w > 0) {
        int rendered_width = running_width;
        running_width += int(src_rect->w);

        // if running too wide, let's back-track and go to the next line
        if (running_width > w) {
          int j = i;
          // back-track to the last space
          while (str[i] != ' ' && i > 0) {
            c = str[i--];
            // subtract the width of each rendered character from running_width
            if (c >= ' ' && c <= '~') {
              src_rect = _font->src_rect + (c - ' ');
              running_width -= int(src_rect->w);
            }
          }
          i++;

          // undo the rendered characters
          if (running_width < rendered_width) {
            SDL_FRect undo_rect = {float(x + running_width), float(_y),
                                   float(rendered_width - running_width),
                                   float(line_height)};
            SDL_RenderFillRect(renderer, &undo_rect);
          }

          // go to next line
          _x = x;
          _y += line_height;
          running_width = 0;

          // render the previously rendered characters
          while (i < j) {
            c = str[i++];
            src_rect = _font->src_rect + (c - ' ');
            running_width += int(src_rect->w);
            dst_rect = {float(_x), float(_y), src_rect->w, src_rect->h};
            _x += int(src_rect->w);
            SDL_RenderTexture(renderer, _font->texture, src_rect, &dst_rect);
          }

          i--;  // continue will increment i
          continue;
        }
      }

      // render character
      dst_rect = {float(_x), float(_y), src_rect->w, src_rect->h};
      SDL_RenderTexture(renderer, _font->texture, src_rect, &dst_rect);
      _x += int(src_rect->w);
    } else {
      switch (c) {
        case '\n':
          _x = x;
          _y += line_height;
          running_width = 0;
          break;
        case '\t': {
          int tabline = ((_x / 32) + 1) * 32;
          int rem = tabline - _x;
          running_width += rem;
          _x += rem;
        } break;
        default: /* pass */
          break;
      }
    }
  }

  SDL_SetRenderTarget(renderer, this->screen);
}

void Game::draw_icon(SDL_Texture* texture, SDL_FRect icon,
                     SDL_FRect* dst_rect) {
  SDL_SetRenderTarget(renderer, texture);

  SDL_RenderTexture(renderer, this->icons, &icon, dst_rect);

  SDL_SetRenderTarget(renderer, this->screen);
}

#define ITEM_NONE "item_none"

void Game::draw_hud(SDL_Texture* texture, std::vector<Game::HudItem> items,
                    int sel_item, int health, int max_health) {
  static SDL_FRect item_list_rect = {0.0f, 0.0f, 0.0f, 0.0f};

  // container ui box
  SDL_FRect hud_rect = {HUD_MAIN_X, HUD_MAIN_Y, 18.0f, 48.0f};
  this->draw_ui_box(texture, BOX_MENU_CONT, &hud_rect);
  this->draw_text(texture, "HP:", SMALL_FONT, HUD_MAIN_X + 5, HUD_MAIN_Y + 4,
                  0);

  SDL_FRect health_rect = {(float)(HUD_MAIN_X + 5), (float)(HUD_MAIN_Y + 11),
                           8.0f, 32.0f};
  int fill_px = ceil(30.0f * (float)health / (float)max_health) + 0.5f;
  SDL_FRect fill_rect = {(float)(HUD_MAIN_X + 6),
                         (float)(HUD_MAIN_Y + 12 + 30 - fill_px), 6.0f,
                         (float)fill_px};
  this->draw_rect(texture, &health_rect, 52, 48, 48, 255, SDL_BLENDMODE_NONE);
  this->draw_rect(texture, &fill_rect, 240, 80, 64, 255, SDL_BLENDMODE_NONE);

  std::string current_item = ITEM_NONE;
  int item_count = 1;

  if (sel_item >= 0 && sel_item < items.size()) {
    current_item = items[sel_item].item_id;
    item_count = items[sel_item].count;
  }

  SDL_FRect item_src_rect = {0.0f, 0.0f, 16.0f, 16.0f};
  SDL_Texture* item_texture;

  // clear last item listing (possibly a different size, if a new item was
  // picked up intermittently)
  SDL_FRect clear_rect = item_list_rect;
  clear_rect.y -= 4.0f;
  clear_rect.h += 10.0f;
  this->draw_rect(texture, &clear_rect, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

  item_list_rect.x = HUD_ITEMS_X;
  item_list_rect.y = HUD_ITEMS_Y;
  item_list_rect.w = float((items.size() + 1) * 18 + 8);
  item_list_rect.h = 24.0f;

  this->draw_ui_box(texture, BOX_CONTAINER, &item_list_rect);

  int sel_offset = HUD_ITEMS_X + 5 + 18 * (sel_item + 1);
  SDL_FRect sel_outline_rect = {(float)(sel_offset - 1),
                                (float)(HUD_ITEMS_Y + 3), 18.0f, 18.0f};
  this->draw_ui_box(texture, BOX_OUT_SEL, &sel_outline_rect);

  for (int i = -1; i < (int)items.size(); i++) {
    int x_offset = HUD_ITEMS_X + 5 + 18 * (i + 1);
    SDL_FRect item_rect = {(float)x_offset, (float)(HUD_ITEMS_Y + 4), 16.0f,
                           16.0f};

    std::string item_id = i < 0 ? ITEM_NONE : items[i].item_id;
    item_texture = load_bmp_texture("sprites/" + item_id + ".bmp");
    SDL_SetRenderTarget(renderer, texture);
    SDL_RenderTexture(renderer, item_texture, &item_src_rect, &item_rect);
    SDL_SetRenderTarget(renderer, this->screen);

    SDL_FRect hint_rect = {float(x_offset), HUD_ITEMS_Y + 15, 5.0f, 7.0f};
    // this->draw_rect(texture, &hint_rect, 24, 24, 24, 128,
    // SDL_BLENDMODE_BLEND);
    game->draw_text(texture, std::to_string(i + 2), SMALL_FONT, x_offset,
                    HUD_ITEMS_Y + 15, 0);

    int this_item_count = i >= 0 ? items[i].count : 1;
    if (this_item_count > 1) {
      SDL_FRect item_count_icon = {(float)(x_offset + 8),
                                   (float)(HUD_ITEMS_Y - 4), 16.0f, 16.0f};
      this->draw_icon(texture,
                      i == sel_item ? ITEM_COUNT_ICON : ITEM_COUNT_ICON_SHD,
                      &item_count_icon);
      this->draw_text(texture, std::to_string(this_item_count), SMALL_FONT,
                      this_item_count > 9 ? x_offset + 11 : x_offset + 13,
                      HUD_ITEMS_Y - 1, 0);
    }
  }
}

// static SDL_FRect _title_rect = SDL_FRect{ 0.0f, 20.0f, 320.0f, 40.0f };
static NetworkAgent::State _last_state = NetworkAgent::State::NO_CONNECTION;
static std::string _last_code = "";
static Uint64 _last_drawn_ticks = 0;
static SDL_FRect _netagent_rect = SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};
static SDL_FRect _neticon_rect = SDL_FRect{10.0f, 8.0f, 16.0f, 16.0f};
static bool _displaying_netagent = false;
static bool _displaying_notification = false;
static bool _displaying_controls_menu = false;
static bool _second_pass = false;
static int _sel_control = -3;
static bool _waiting_for_key = false;

void Game::draw_overlay() {
  bool force_render = false;

  if (this->display_controls_menu) {
    bool should_render = !_displaying_controls_menu;
    _displaying_controls_menu = true;

    if (_waiting_for_key) {
      if (captured_controller.last_input != SDLK_UNKNOWN) {
        SDL_Keycode new_key = captured_controller.last_input;
        Button sel_button = (Button)_sel_control;

        if (ktobutton_map.find(new_key) != ktobutton_map.end()) {
          SDL_Keycode old_key = btokeycode_map[sel_button];
          Button existing_button = ktobutton_map[new_key];

          btokeycode_map[existing_button] = old_key;
          ktobutton_map[old_key] = existing_button;
        } else {
          SDL_Keycode old_key = btokeycode_map[sel_button];
          ktobutton_map.erase(old_key);
        }

        btokeycode_map[sel_button] = new_key;
        ktobutton_map[new_key] = sel_button;

        should_render = true;
        _waiting_for_key = false;
      }
    } else {
      int y_dir =
          captured_controller.is_hit(DOWN) - captured_controller.is_hit(UP);
      int new_sel_control =
          cnf_clamp(_sel_control + y_dir, -3, (int)Button::DIGIT - 1);

      if (captured_controller.is_hit(Button::SELECT) && _sel_control == -3) {
        game->map = "maps/init";
        save.clear();

        // return from this menu
        capture_controls = false;
        this->display_controls_menu = false;
        captured_controller.clear_all();

        return;
      }

      int x_dir =
          captured_controller.is_hit(RIGHT) - captured_controller.is_hit(LEFT);
      if (_sel_control == -2 && x_dir != 0) {
        game->volume = cnf_clamp(game->volume + (10 * x_dir), 0, 200);
        should_render = true;
      } else if (_sel_control == -1 && x_dir != 0) {
        game->music_volume =
            cnf_clamp(game->music_volume + (10 * x_dir), 0, 200);
        should_render = true;
      }

      if (_sel_control != new_sel_control) {
        _sel_control = new_sel_control;
        should_render = true;
      } else if (captured_controller.is_hit(Button::SELECT) &&
                 _sel_control >= 0) {
        captured_controller.last_input = SDLK_UNKNOWN;
        _waiting_for_key = true;
        should_render = true;
      }
    }

    if (should_render) {
      this->draw_rect(this->overlay, nullptr, this->bg_r, this->bg_g,
                      this->bg_b, 128, SDL_BLENDMODE_NONE);

      SDL_FRect config_rect = {8.0f, 8.0f, 112.0f, 168.0f};
      this->draw_ui_box(this->overlay, BOX_CONTAINER, &config_rect);

      this->draw_text(this->overlay, "Game", SMALL_FONT, 16, 16, 0);

      game->draw_text(this->overlay, "Exit to Main Menu", SMALL_FONT, 20, 28,
                      0);
      if (_sel_control != -3) {
        SDL_FRect option_rect = {20.0f, 28.0f, 96.0f, 8.0f};
        game->draw_rect(this->overlay, &option_rect, 24, 24, 24, 96,
                        SDL_BLENDMODE_BLEND);
      }

      this->draw_text(this->overlay, "Volume Settings", SMALL_FONT, 16, 40, 0);

      game->draw_text(this->overlay, "Sounds", SMALL_FONT, 20, 52, 0);
      game->draw_text(this->overlay, std::to_string(game->volume), SMALL_FONT,
                      52, 52, 0);
      if (_sel_control != -2) {
        SDL_FRect sound_rect = {20.0f, 52.0f, 96.0f, 8.0f};
        game->draw_rect(this->overlay, &sound_rect, 24, 24, 24, 96,
                        SDL_BLENDMODE_BLEND);
        SDL_FRect volume_rect = {52.0f, 52.0f, 64.0f, 8.0f};
        game->draw_rect(this->overlay, &volume_rect, 24, 24, 24, 128,
                        SDL_BLENDMODE_BLEND);
      } else {
        SDL_FRect volume_rect = {52.0f, 52.0f, 64.0f, 8.0f};
        game->draw_rect(this->overlay, &volume_rect, 24, 24, 24, 96,
                        SDL_BLENDMODE_BLEND);
      }

      game->draw_text(this->overlay, "Music", SMALL_FONT, 20, 60, 0);
      game->draw_text(this->overlay, std::to_string(game->music_volume),
                      SMALL_FONT, 52, 60, 0);
      if (_sel_control != -1) {
        SDL_FRect music_rect = {20.0f, 60.0f, 96.0f, 8.0f};
        game->draw_rect(this->overlay, &music_rect, 24, 24, 24, 96,
                        SDL_BLENDMODE_BLEND);
        SDL_FRect volume_rect = {52.0f, 60.0f, 64.0f, 8.0f};
        game->draw_rect(this->overlay, &volume_rect, 24, 24, 24, 128,
                        SDL_BLENDMODE_BLEND);
      } else {
        SDL_FRect volume_rect = {52.0f, 60.0f, 64.0f, 8.0f};
        game->draw_rect(this->overlay, &volume_rect, 24, 24, 24, 96,
                        SDL_BLENDMODE_BLEND);
      }

      this->draw_text(this->overlay, "Controls", SMALL_FONT, 16, 72, 0);

      int start_y = 84;

      for (int i = 0; i < (int)Button::DIGIT; i++) {
        std::string control_name = btostring_map[(Button)i];
        std::string input_name =
            _waiting_for_key && i == _sel_control
                ? "<Press a Key>"
                : SDL_GetKeyName(btokeycode_map[(Button)i]);

        int y = start_y + (i * 8);
        game->draw_text(this->overlay, control_name, SMALL_FONT, 20, y, 0);
        game->draw_text(this->overlay, input_name, SMALL_FONT, 52, y, 0);

        int control_opacity = 128;
        int input_opacity = 192;
        if (i == _sel_control) {
          if (_waiting_for_key) {
            control_opacity = 96;
            input_opacity = 0;
          } else {
            control_opacity = 0;
            input_opacity = 96;
          }
        }

        SDL_FRect control_rect = {20.0f, (float)y, 28.0f, 8.0f};
        SDL_FRect input_rect = {52.0f, (float)y, 64.0f, 8.0f};
        game->draw_rect(this->overlay, &control_rect, 24, 24, 24,
                        control_opacity, SDL_BLENDMODE_BLEND);
        game->draw_rect(this->overlay, &input_rect, 24, 24, 24, input_opacity,
                        SDL_BLENDMODE_BLEND);
      }
    }

    return;  // don't display anything else
  } else if (_displaying_controls_menu) {
    this->draw_rect(this->overlay, nullptr, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
    _displaying_controls_menu = false;
    force_render = true;
  }

  if (this->current_map == "maps/splash" || this->current_map == "maps/init" ||
      this->current_map == "maps/dead" || this->current_map == "maps/credits") {
    if (!force_render) {
      force_render = true;
      this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
    }

    return;
  }

  // network agent overlay (only cheddar can "create objects" so that is used to
  // check if cheddar)
  if (game->create_objects && net_agent != nullptr) {
    std::string code = net_agent->get_connection_code();

    // draw if state changed, code changed, or if a second has passed since last
    // rendered
    if (force_render || this->net_state != _last_state || code != _last_code ||
        this->ticks - _last_drawn_ticks > 1000) {
      // clear last overlay
      if (_netagent_rect.w > 0.0f) {
        this->draw_rect(this->overlay, &_netagent_rect, 0, 0, 0, 0,
                        SDL_BLENDMODE_NONE);
      }

      switch (this->net_state) {
        case NetworkAgent::State::NO_CONNECTION:
          _netagent_rect = SDL_FRect{8.0f, 8.0f, 88.0f, 16.0f};
          this->draw_ui_box(this->overlay, BOX_MENU_CONT, &_netagent_rect);
          this->draw_icon(this->overlay, NOT_CONNECTED_ICON, &_neticon_rect);
          this->draw_text(this->overlay, "Not Connected", DEFAULT_FONT, 28, 12,
                          0);
          _displaying_netagent = true;
          break;
        case NetworkAgent::State::WAITING_FOR_PEER:
          _netagent_rect = SDL_FRect{8.0f, 8.0f, 80.0f, 16.0f};
          this->draw_ui_box(this->overlay, BOX_MENU_CONT, &_netagent_rect);
          this->draw_icon(this->overlay, WAITING_FOR_PEER_ICON, &_neticon_rect);
          this->draw_text(this->overlay, "Code:", SMALL_FONT, 28, 13, 0);
          this->draw_text(this->overlay, code, CODE_FONT, 48, 12, 0);
          _displaying_netagent = true;
          break;
        default:
          // clear entire overlay screen
          this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
          _netagent_rect = SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};  // no dst rect
          _displaying_netagent = false;
          _displaying_notification = false;
          break;  // display nothing
      }

      _last_state = this->net_state;
      _last_code = code;
      _last_drawn_ticks = this->ticks;
    }
  }

  if (!this->notification.empty()) {
    if (force_render || this->force_notif_rerender) {
      this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
      _displaying_notification = false;
      this->force_notif_rerender = false;

      if (_displaying_netagent && !_second_pass) {
        _last_drawn_ticks = 0;
        _second_pass = true;
        this->draw_overlay();
        _second_pass = false;
      }
    }

    if (!_displaying_notification) {
      int text_w = fonts[DEFAULT_FONT]->text_width(this->notification);
      SDL_FRect notif_rect = SDL_FRect{8.0f, 8.0f, (float)(text_w + 12), 16.0f};
      int text_y = 12;

      if (_displaying_netagent) {
        notif_rect.y = 26.0f;
        text_y = 30;
      }

      this->draw_ui_box(this->overlay, BOX_MENU_CONT, &notif_rect);
      this->draw_text(this->overlay, this->notification, DEFAULT_FONT, 14,
                      text_y, 0);

      _displaying_notification = true;
    } else if (this->ticks - this->notif_ticks > 3000) {
      this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
      this->notification.clear();
      _displaying_notification = false;

      if (_displaying_netagent && !_second_pass) {
        _last_drawn_ticks = 0;
        _second_pass = true;
        this->draw_overlay();
        _second_pass = false;
      }
    }
  }
}
