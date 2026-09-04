#include "game_over.h"

#include "controller.h"
#include "save_data.h"

GameOver::GameOver() {
  this->ui = SDL_CreateTexture(thoom::renderer, SDL_PIXELFORMAT_RGBA32,
                               SDL_TEXTUREACCESS_TARGET, THOOM_SCREEN_WIDTH,
                               THOOM_SCREEN_HEIGHT);

  // SDL_FRect ui_box = { 112.0f, 176.0f, 96.0f, 32.0f };
  // game->draw_ui_box(this->ui, BOX_CONTAINER, &ui_box);
  thoom::game->draw_text(this->ui, "(Press [Enter] to try again)", DEFAULT_FONT,
                         98, 160, 0);
  this->src = this->dst = {0.0f, 0.0f, (float)THOOM_SCREEN_WIDTH,
                           (float)THOOM_SCREEN_HEIGHT};

  thoom::game->corner_x = 0;
  thoom::game->corner_y = 0;
}

GameOver::~GameOver() { SDL_DestroyTexture(this->ui); }

void GameOver::step() {
  thoom::game->push_sprite("", this->ui, &this->src, &this->dst, 0);

  if (thoom::local_controller.is_hit(thoom::Button::SELECT)) {
    thoom::game->map = "maps/init";
    thoom::save.clear();
  }
}