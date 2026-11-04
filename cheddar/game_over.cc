#include "game_over.h"

#include "constants.h"
#include "controller.h"
#include "renderer.h"
#include "save_data.h"
#include "texture.h"

GameOver::GameOver() {
  this->ui = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_LINEAR);

  // SDL_FRect ui_box = { 112.0f, 176.0f, 96.0f, 32.0f };
  thoom::Renderer::instance->draw_text(this->ui, DEFAULT_FONT,
                                       "(Press [Enter] to try again)", 98, 160,
                                       0, kBackground);
  this->src = this->dst = {0.0f, 0.0f, (float)THOOM_SCREEN_WIDTH,
                           (float)THOOM_SCREEN_HEIGHT};

  thoom::Renderer::instance->reset_view();
}

GameOver::~GameOver() { SDL_DestroyTexture(this->ui); }

void GameOver::step() {
  thoom::Renderer::instance->push_sprite("", this->ui, &this->src, &this->dst,
                                         0);

  if (thoom::local_controller.is_hit(thoom::Button::SELECT)) {
    thoom::game->next_map = "maps/init";
    thoom::save.clear();
  }
}
