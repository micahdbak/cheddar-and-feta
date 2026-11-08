#include "init.h"

#include <string>

#include "colour.h"
#include "constants.h"
#include "guest_game.h"
#include "host_game.h"
#include "input/controller.h"
#include "input/input.h"
#include "object.h"
#include "peer/peer.h"
#include "renderer/font.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "texture/texture.h"
#include "thoom/macros.h"
#include "window/window.h"

static bool host_mode = false;

bool init_show_peer(void* user) { return host_mode; }

Init::Init() {
  this->ui = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_LINEAR);
  SDL_SetTextureBlendMode(this->ui, SDL_BLENDMODE_BLEND);
  thoom::Renderer::instance->reset_view();
}

Init::~Init() { SDL_DestroyTexture(this->ui); }

void Init::draw_choice() {
  thoom::Renderer::instance->clear(this->ui, thoom::kMask);

  int host_w =
      thoom::Renderer::instance->fonts[DEFAULT_FONT]->text_width("Host");
  int join_w =
      thoom::Renderer::instance->fonts[DEFAULT_FONT]->text_width("Join");

  SDL_FRect host_box = {0.0f, 112.0f, (float)(host_w + 8), 16.0f};
  SDL_FRect join_box = {0.0f, 112.0f, (float)(join_w + 8), 16.0f};

  const float gap = 8.0f;
  float total_w = host_box.w + gap + join_box.w;
  host_box.x = 160.0f - total_w / 2.0f;
  join_box.x = host_box.x + host_box.w + gap;

  SDL_FRect menu_box = {host_box.x - 4.0f, host_box.y - 4.0f, total_w + 8.0f,
                        24.0f};
  thoom::Renderer::instance->draw_ui_box(UI_BOX_CHAR_CONT, this->ui, &menu_box);

  thoom::Renderer::instance->draw_ui_box(
      this->menu_sel == 0 ? UI_BOX_CHAR_SEL : UI_BOX_CHAR_BOX, this->ui,
      &host_box);
  thoom::Renderer::instance->draw_text(
      this->ui, DEFAULT_FONT, "Host", (int)(host_box.x + 4.0f),
      (int)(host_box.y + 3.0f), 0, kBackground);

  thoom::Renderer::instance->draw_ui_box(
      this->menu_sel == 1 ? UI_BOX_CHAR_SEL : UI_BOX_CHAR_BOX, this->ui,
      &join_box);
  thoom::Renderer::instance->draw_text(
      this->ui, DEFAULT_FONT, "Join", (int)(join_box.x + 4.0f),
      (int)(join_box.y + 3.0f), 0, kBackground);
}

void Init::step() {
  if (this->which == kChoose) {
    int x_dir = thoom::Input::instance->world.consume_hit_dx();
    if (x_dir != 0) {
      this->menu_sel = THOOM_CLAMP(this->menu_sel + x_dir, 0, 1);
      this->render = true;
    }

    if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
      if (this->menu_sel == 0) {
        host_mode = true;
        thoom::Peer::instance->set_offerer(false);
        host_start();
        this->init_host.start(this->ui);
        this->which = kHost;
      } else {
        host_mode = false;
        thoom::Peer::instance->set_offerer(true);
        guest_start();
        this->init_join.start(this->ui);
        this->which = kJoin;
      }
    } else if (this->render) {
      this->render = false;
      this->draw_choice();
    }
  }

  if (this->which == kHost) {
    this->init_host.step();
  } else if (this->which == kJoin) {
    this->init_join.step();
  }

  thoom::Renderer::instance->push_sprite("", this->ui, nullptr, &this->dst_rect,
                                         0);
}

thoom::Object* Init::create_object(thoom::Scene* scene,
                                   const std::string& options) {
  return new Init();
}
