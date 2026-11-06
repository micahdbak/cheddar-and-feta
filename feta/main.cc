#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "audio.h"
#include "colour.h"
#include "constants.h"
#include "controller.h"
#include "feta.h"
#include "font.h"
#include "hud.h"
#include "icon.h"
#include "input.h"
#include "item.h"
#include "map.h"
#include "object.h"
#include "peer.h"
#include "peer_receiver.h"
#include "peer_sender.h"
#include "render_functions.h"
#include "renderer.h"
#include "scene.h"
#include "sounds.h"
#include "texture.h"
#include "thoom/macros.h"
#include "thoom/thoom.h"
#include "ui_box.h"
#include "window.h"

#define INIT_OBJ "load_save"

bool game_is_cheddar = false;

std::unordered_map<std::string, Item> item_info;

bool init(int argc, char** argv);

int main(int argc, char** argv) {
  if (!thoom_init("Thoom Engine", THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT,
                  true)) {
    return 1;
  }

  register_render_functions();

  if (!thoom::Scene::init()) {
    thoom_quit();
    return 1;
  }

  if (!init(argc, argv)) {
    thoom::Scene::quit();
    thoom_quit();
    return 1;
  }

  load_ui_box();
  load_icons();
  Hud::init();

  load_sounds();

  bool running = true;

  while (running) {
    bool quit = false;
    thoom_poll_events(&quit);

    if (quit) {
      running = false;
      break;
    }

    thoom::Audio::instance->step();
    thoom::Scene::instance->step();

    thoom::Map* map = thoom::Scene::instance->map;
    thoom::Colour bg = map != nullptr ? map->background : thoom::kBlack;
    thoom::Window::instance->clear_screen(bg);
    thoom::Renderer::instance->draw_map_background(map);
    thoom::Renderer::instance->draw_sprites();
    thoom::Renderer::instance->draw_map_foreground(map);

    if (thoom::Input::instance->world.character == 'p') {
      thoom::Window::instance->save_screenshot("screenshot.bmp");
    }

    Hud::instance->render();

    thoom::Renderer::instance->draw_texture(thoom::Window::instance->screen,
                                            Hud::instance->ui, NULL, NULL);
    thoom::Renderer::instance->draw_texture(thoom::Window::instance->screen,
                                            Hud::instance->overlay, NULL, NULL);

    thoom::Input::instance->clear_hit_input();

    thoom::Window::instance->present(bg);
  }

  Hud::shutdown();

  thoom::Scene::quit();

  SDL_DestroyTexture(ui_box);
  ui_box = nullptr;

  SDL_DestroyTexture(icons);
  icons = nullptr;

  thoom_quit();

  return 0;
}

class Init : public thoom::Object {
 public:
  Init() {
    this->ui = thoom::Texture::instance->create_texture(
        THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
        SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(this->ui, SDL_BLENDMODE_BLEND);
    thoom::Renderer::instance->reset_view();
  }

  ~Init() { SDL_DestroyTexture(this->ui); }

  void step();

  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options) {
    return new Init();
  }

 private:
  bool render = true, waiting_for_peer = false;
  int sel_c = 0;
  std::string code;
  SDL_Texture* ui = nullptr;
  SDL_FRect dst_rect = {0.0f, 0.0f, 320.0f, 240.0f};
  uint64_t last_peer_state = thoom::Peer::kStateNoConnection;
  Uint64 waiting_ticks = 0;
};

static const thoom::Scene::ObjectDef kFetaObjects[] = {
    {INIT_OBJ, Init::create_object},
    {PEER_RECEIVER_OBJ, PeerReceiver::create_object, thoom::kObjectFirst},
    {PEER_SENDER_OBJ, PeerSender::create_object, thoom::kObjectLast},
    {FETA_OBJ, Feta::create_object},
    {nullptr, nullptr},
};

bool init(int argc, char** argv) {
  thoom::Scene::instance->register_objects(kFetaObjects);

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

  thoom::Window::instance->set_title("Playing as Feta");

  thoom::Scene::instance->create_object(PEER_RECEIVER_OBJ, "");
  thoom::Scene::instance->create_object(PEER_SENDER_OBJ, "");

  thoom::Scene::instance->request_map("maps/init");
  return true;
}

void Init::step() {
  uint64_t peer_state = thoom::Peer::instance->state.load();

  if (peer_state != this->last_peer_state) this->render = true;

  this->last_peer_state = peer_state;

  if (this->waiting_for_peer) {
    if (peer_state == thoom::Peer::kStateNoConnection)
      peer_state = thoom::Peer::kStateWaitingForPeer;
    else {
      this->waiting_for_peer = false;
      this->waiting_ticks = thoom::Scene::instance->ticks;
    }
  }

  switch (peer_state) {
    case thoom::Peer::kStateNoConnection: {
      static char code_charset[38] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ*";
      static const int cols = 10;
      static const int rows = 4;
      bool valid_code = this->code.size() == 6;

      int x_dir = thoom::Input::instance->world.consume_hit_dx();
      int y_dir = thoom::Input::instance->world.consume_hit_dy();
      bool select =
          thoom::Input::instance->world.consume_hit(thoom::Button::SELECT);
      bool remove =
          thoom::Input::instance->world.consume_hit(thoom::Button::CANCEL);

      this->sel_c += x_dir;
      this->sel_c += y_dir * cols;
      this->sel_c = THOOM_CLAMP(this->sel_c, 0, valid_code ? 36 : 35);

      if (x_dir != 0 || y_dir != 0 || select || remove) {
        this->render = true;
      }

      if (this->render) {
        this->render = false;

        thoom::Renderer::instance->clear(this->ui, thoom::kMask);

        SDL_FRect code_ui_box = {80.0f, 64.0f, 160.0f, 32.0f};
        draw_ui_box(UI_BOX_CONTAINER, this->ui, &code_ui_box);

        std::string prompt = "Please enter the connection code.";
        int prompt_w =
            thoom::Renderer::instance->fonts[DEFAULT_FONT]->text_width(prompt);
        thoom::Renderer::instance->draw_text(this->ui, DEFAULT_FONT, prompt,
                                             160 - prompt_w / 2, 70, 0,
                                             kBackground);

        thoom::Renderer::instance->draw_text(this->ui, SMALL_FONT, "Code:", 130,
                                             82, 0, kBackground);
        thoom::Renderer::instance->draw_text(this->ui, CODE_FONT, this->code,
                                             154, 81, 0, kBackground);

        SDL_FRect ui_box = {80.0f, 104.0f, 160.0f, 64.0f};
        draw_ui_box(UI_BOX_CHAR_CONT, this->ui, &ui_box);

        const int start_x = 100;
        const int start_y = 112;

        for (int i = 0; i < 36; i++) {
          int x = start_x + (i % cols) * 12;
          int y = start_y + (i / cols) * 12;

          SDL_FRect box_rect = {(float)x, (float)y, 12.0f, 12.0f};
          draw_ui_box(i == this->sel_c ? UI_BOX_CHAR_SEL : UI_BOX_CHAR_BOX,
                      this->ui, &box_rect);
          char c = code_charset[i];
          SDL_FRect src_rect =
              thoom::Renderer::instance->fonts[CODE_FONT]->src_rect[c - ' '];

          box_rect.x = (float)(int)(x + 6 - (int)src_rect.w / 2);
          box_rect.y = (float)(int)(y + 6 - (int)src_rect.h / 2);
          box_rect.w = src_rect.w;
          box_rect.h = src_rect.h;

          thoom::Renderer::instance->draw_texture(
              this->ui,
              thoom::Renderer::instance
                  ->fonts[i == this->sel_c ? CODE_FONT : CODE_GRAY_FONT]
                  ->texture,
              &src_rect, &box_rect);
        }

        if (valid_code) {
          SDL_FRect enter_box = {172.0f, 147.0f, 18.0f, 12.0f};
          draw_ui_box(36 == this->sel_c ? UI_BOX_CHAR_SEL : UI_BOX_CHAR_BOX,
                      this->ui, &enter_box);
          thoom::Renderer::instance->draw_text(
              this->ui, 36 == this->sel_c ? CODE_FONT : CODE_GRAY_FONT, "OK",
              175, 148, 0, kBackground);
        }

        if (this->sel_c == 36) {
          if (select && valid_code) {
            thoom::Peer::instance->set_connection_code(this->code);
            this->waiting_for_peer = true;
            this->waiting_ticks = thoom::Scene::instance->ticks;
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

        thoom::Renderer::instance->clear(this->ui, thoom::kMask);

        SDL_FRect waiting_box = {76.0f, 100.0f, 168.0f, 40.0f};
        draw_ui_box(UI_BOX_CHAR_CONT, this->ui, &waiting_box);

        SDL_FRect waiting_disp_box = {84.0f, 108.0f, 152.0f, 24.0f};
        draw_ui_box(UI_BOX_CHAR_DISP, this->ui, &waiting_disp_box);

        std::string waiting = "Connecting with code " + this->code + "...";
        if (peer_state == thoom::Peer::kStateConnected) {
          waiting = "Waiting for Cheddar...";
        }

        int waiting_w =
            thoom::Renderer::instance->fonts[DEFAULT_FONT]->text_width(waiting);
        thoom::Renderer::instance->draw_text(this->ui, DEFAULT_FONT, waiting,
                                             160 - waiting_w / 2, 116, 0,
                                             kBackground);
      }

      // no connection after 15 seconds
      if (thoom::Scene::instance->ticks - this->waiting_ticks > 15000) {
        thoom::Peer::instance->state.store(thoom::Peer::kStateTryReset);
        Hud::instance->display_notification("That code didn't work.");
        this->waiting_ticks = 0;
      }

      break;
  }

  thoom::Renderer::instance->push_sprite("", this->ui, nullptr, &this->dst_rect,
                                         0);
}
