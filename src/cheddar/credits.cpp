#include "bmp_texture.h"
#include "controller.h"
#include "credits.h"
#include "game.h"
#include "save_data.h"

Credits::Credits() {
    this->tex_id = RENDER_CREDITS + credits_args(1, 1, 1, 1, 60);
    this->texture = load_bmp_texture(tex_id);

    this->src_rect = this->dst_rect = SDL_FRect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    game->corner_x = 0;
    game->corner_y = 0;
}

void Credits::step() {
    game->push_sprite(this->tex_id, this->texture, &this->src_rect, &this->dst_rect, 0);

    if (local_controller.is_hit(Button::SELECT)) {
        game->map = "maps/init";
        save.clear();
    }
}