#include <iostream>
#include <string>

#include "controller.h"
#include "feta.h"
#include "font.h"
#include "game.h"
#include "item.h"
#include "net_agent.h"
#include "net_receiver.h"
#include "net_sender.h"
#include "utils.h"

#define INIT_OBJ "load_save"

std::unordered_map<std::string, Item> item_info;

class Init : public thoom::Object {
 public:
  Init() {
    this->ui = SDL_CreateTexture(thoom::renderer, SDL_PIXELFORMAT_RGBA32,
                                 SDL_TEXTUREACCESS_TARGET, THOOM_SCREEN_WIDTH,
                                 THOOM_SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(this->ui, SDL_BLENDMODE_BLEND);
    thoom::game->set_view(160, 120);
  }

  ~Init() { SDL_DestroyTexture(this->ui); }

  void step();

 private:
  bool render = true, waiting_for_net_agent = false;
  int sel_c = 0;
  std::string code;
  SDL_Texture* ui = nullptr;
  SDL_FRect dst_rect = {0.0f, 0.0f, 320.0f, 240.0f};
  thoom::NetworkAgent::State last_net_state =
      thoom::NetworkAgent::State::NO_CONNECTION;
  Uint64 waiting_ticks = 0;
};

class InitFactory : public thoom::ObjectFactory {
 public:
  InitFactory() = default;
  ~InitFactory() = default;

  thoom::Object* create(const std::string& options) override {
    return new Init();
  }
};

void thoom::Game::init() {
  this->factories[INIT_OBJ] = new InitFactory();

  this->factories[FIRST_OBJ] = new NetReceiverFactory();
  this->factories[LAST_OBJ] = new NetSenderFactory();

  this->factories[FETA_OBJ] = new FetaFactory();

  // items
  item_info[ITEM_NONE] = Item{HELD_EFFECT, "Kick"};
  item_info[ITEM_NONE].damage = 1;
  item_info[ITEM_NONE].armour = 0;
  item_info[ITEM_CANNON_BALL] = Item{THROWABLE, "Cannon Ball"};
  item_info[ITEM_CHEESE] = Item{EDIBLE, "Cheese"};
  item_info[ITEM_COFFEE_BEAN] = Item{USEFUL, "Coffee Bean"};
  item_info[ITEM_FIRE] = Item{THROWABLE, "Fire"};
  item_info[ITEM_HERMES_BOOT] = Item{HELD_EFFECT, "Hermes Boot"};
  item_info[ITEM_HERMES_BOOT].damage = 0;
  item_info[ITEM_HERMES_BOOT].speed = 1.75f;
  item_info[ITEM_MOLOTOV] = Item{THROWABLE, "Molotov Cocktail"};
  item_info[ITEM_SHIELD] = Item{HELD_EFFECT, "Shield"};
  item_info[ITEM_SHIELD].damage = 0;
  item_info[ITEM_SHIELD].armour = 1;
  item_info[ITEM_TOOTHPICK] = Item{THROWABLE, "Porcu' Pine"};

  this->title = "Playing as Feta";

  this->load_map("maps/init");
  this->create_objects = false;

  thoom::net_agent = new thoom::NetworkAgent(true);

  this->create_object(FIRST_OBJ, "");
  this->create_object(LAST_OBJ, "");
}

void Init::step() {
  thoom::NetworkAgent::State net_state = thoom::net_agent->get_state();

  if (net_state != this->last_net_state) this->render = true;

  this->last_net_state = net_state;

  if (this->waiting_for_net_agent) {
    if (net_state == thoom::NetworkAgent::State::NO_CONNECTION)
      net_state = thoom::NetworkAgent::State::WAITING_FOR_PEER;
    else {
      this->waiting_for_net_agent = false;
      this->waiting_ticks = thoom::game->ticks;
    }
  }

  switch (net_state) {
    case thoom::NetworkAgent::State::NO_CONNECTION: {
      static char code_charset[38] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ*";
      static const int cols = 10;
      static const int rows = 4;
      bool valid_code = this->code.size() == 6;

      int x_dir = thoom::local_controller.is_hit(thoom::Button::RIGHT) -
                  thoom::local_controller.is_hit(thoom::Button::LEFT);
      int y_dir = thoom::local_controller.is_hit(thoom::Button::DOWN) -
                  thoom::local_controller.is_hit(thoom::Button::UP);
      bool select = thoom::local_controller.is_hit(thoom::Button::SELECT);
      bool remove = thoom::local_controller.is_hit(thoom::Button::CANCEL);

      this->sel_c += x_dir;
      this->sel_c += y_dir * cols;
      this->sel_c = THOOM_CLAMP(this->sel_c, 0, valid_code ? 36 : 35);

      if (x_dir != 0 || y_dir != 0 || select || remove) {
        this->render = true;
      }

      if (this->render) {
        this->render = false;

        thoom::game->draw_rect(this->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

        SDL_FRect code_ui_box = {80.0f, 64.0f, 160.0f, 32.0f};
        thoom::game->draw_ui_box(this->ui, BOX_CONTAINER, &code_ui_box);

        std::string prompt = "Please enter the connection code.";
        int prompt_w = thoom::game->fonts[DEFAULT_FONT]->text_width(prompt);
        thoom::game->draw_text(this->ui, prompt, DEFAULT_FONT,
                               160 - prompt_w / 2, 70, 0);

        thoom::game->draw_text(this->ui, "Code:", SMALL_FONT, 130, 82, 0);
        thoom::game->draw_text(this->ui, this->code, CODE_FONT, 154, 81, 0);

        SDL_FRect ui_box = {80.0f, 104.0f, 160.0f, 64.0f};
        thoom::game->draw_ui_box(this->ui, BOX_CHAR_CONT, &ui_box);

        const int start_x = 100;
        const int start_y = 112;

        for (int i = 0; i < 36; i++) {
          int x = start_x + (i % cols) * 12;
          int y = start_y + (i / cols) * 12;

          SDL_FRect box_rect = {(float)x, (float)y, 12.0f, 12.0f};
          thoom::game->draw_ui_box(
              this->ui, i == this->sel_c ? BOX_CHAR_SEL : BOX_CHAR_BOX,
              &box_rect);
          char c = code_charset[i];
          SDL_FRect src_rect = thoom::game->fonts[CODE_FONT]->src_rect[c - ' '];

          box_rect.x = (float)(int)(x + 6 - (int)src_rect.w / 2);
          box_rect.y = (float)(int)(y + 6 - (int)src_rect.h / 2);
          box_rect.w = src_rect.w;
          box_rect.h = src_rect.h;

          SDL_SetRenderTarget(thoom::renderer, this->ui);
          SDL_RenderTexture(
              thoom::renderer,
              thoom::game->fonts[i == this->sel_c ? CODE_FONT : CODE_GRAY_FONT]
                  ->texture,
              &src_rect, &box_rect);
          SDL_SetRenderTarget(thoom::renderer, thoom::game->screen);
        }

        if (valid_code) {
          SDL_FRect enter_box = {172.0f, 147.0f, 18.0f, 12.0f};
          thoom::game->draw_ui_box(
              this->ui, 36 == this->sel_c ? BOX_CHAR_SEL : BOX_CHAR_BOX,
              &enter_box);
          thoom::game->draw_text(this->ui, "OK",
                                 36 == this->sel_c ? CODE_FONT : CODE_GRAY_FONT,
                                 175, 148, 0);
        }

        if (this->sel_c == 36) {
          if (select && valid_code) {
            thoom::net_agent->set_connection_code(this->code);
            this->waiting_for_net_agent = true;
            this->waiting_ticks = thoom::game->ticks;
            this->render = true;
          }
        } else if (select && this->code.size() < 6) {
          this->code += code_charset[this->sel_c];
          this->render = true;  // render character on next frame
        }

        if (remove && !this->code.empty()) {
          this->code.pop_back();
          this->render = true;  // remove character on next frame
        }
      }
    } break;

    // WAITING_FOR_PEER, CONNECTION
    default:
      if (this->render) {
        this->render = false;

        thoom::game->draw_rect(this->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

        SDL_FRect waiting_box = {76.0f, 100.0f, 168.0f, 40.0f};
        thoom::game->draw_ui_box(this->ui, BOX_CHAR_CONT, &waiting_box);

        SDL_FRect waiting_disp_box = {84.0f, 108.0f, 152.0f, 24.0f};
        thoom::game->draw_ui_box(this->ui, BOX_CHAR_DISP, &waiting_disp_box);

        std::string waiting = "Connecting with code " + this->code + "...";
        if (net_state == thoom::NetworkAgent::State::CONNECTED) {
          waiting = "Waiting for Cheddar...";
        }

        int waiting_w = thoom::game->fonts[DEFAULT_FONT]->text_width(waiting);
        thoom::game->draw_text(this->ui, waiting, DEFAULT_FONT,
                               160 - waiting_w / 2, 116, 0);
      }

      // no connection after 15 seconds
      if (thoom::game->ticks - this->waiting_ticks > 15000) {
        thoom::net_agent->try_reset();
        thoom::game->display_notification("That code didn't work.");
        this->waiting_ticks = 0;
      }

      break;
  }

  thoom::game->push_sprite("", this->ui, nullptr, &this->dst_rect, 0);
}