#include "game_over.h"
#include "controller.h"
#include "save_data.h"

GameOver::GameOver() {
    this->ui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

    // SDL_FRect ui_box = { 112.0f, 176.0f, 96.0f, 32.0f };
    // game->draw_ui_box(this->ui, BOX_CONTAINER, &ui_box);
    game->draw_text(this->ui, "(Press [Enter] to try again)", DEFAULT_FONT, 98, 160, 0);
    this->src = this->dst = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };

    game->corner_x = 0;
    game->corner_y = 0;
}

GameOver::~GameOver() {
    SDL_DestroyTexture(this->ui);
}

void GameOver::step() {
    game->push_sprite("", this->ui, &this->src, &this->dst, 0);

    if (local_controller.is_hit(Button::SELECT)) {
        game->map = "maps/init";
        save.clear();
    }
}