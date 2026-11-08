#include "init_join.h"

#include <cstdint>
#include <string>

#include "colour.h"
#include "constants.h"
#include "hud/hud.h"
#include "input/controller.h"
#include "input/input.h"
#include "peer/peer.h"
#include "renderer/font.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "thoom/macros.h"

void InitJoin::start(SDL_Texture* ui) {
  this->ui = ui;
  this->render = true;
  this->waiting_for_peer = false;
  this->sel_c = 0;
  this->code.clear();
  this->last_peer_state = thoom::Peer::instance->get_state();
  this->waiting_ticks = 0;
}

void InitJoin::step() {
  uint64_t peer_state = thoom::Peer::instance->get_state();

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
        thoom::Renderer::instance->draw_ui_box(UI_BOX_CONTAINER, this->ui,
                                               &code_ui_box);

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
        thoom::Renderer::instance->draw_ui_box(UI_BOX_CHAR_CONT, this->ui,
                                               &ui_box);

        const int start_x = 100;
        const int start_y = 112;

        for (int i = 0; i < 36; i++) {
          int x = start_x + (i % cols) * 12;
          int y = start_y + (i / cols) * 12;

          SDL_FRect box_rect = {(float)x, (float)y, 12.0f, 12.0f};
          thoom::Renderer::instance->draw_ui_box(
              i == this->sel_c ? UI_BOX_CHAR_SEL : UI_BOX_CHAR_BOX, this->ui,
              &box_rect);
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
          thoom::Renderer::instance->draw_ui_box(
              36 == this->sel_c ? UI_BOX_CHAR_SEL : UI_BOX_CHAR_BOX, this->ui,
              &enter_box);
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
        thoom::Renderer::instance->draw_ui_box(UI_BOX_CHAR_CONT, this->ui,
                                               &waiting_box);

        SDL_FRect waiting_disp_box = {84.0f, 108.0f, 152.0f, 24.0f};
        thoom::Renderer::instance->draw_ui_box(UI_BOX_CHAR_DISP, this->ui,
                                               &waiting_disp_box);

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
        thoom::Peer::instance->set_state(thoom::Peer::kStateTryReset);
        thoom::Hud::instance->display_notification("That code didn't work.");
        this->waiting_ticks = 0;
      }

      break;
  }
}
