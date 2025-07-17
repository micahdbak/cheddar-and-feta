#include "ladder.h"
#include "game.h"
#include "mouse.h"
#include "save_data.h"

Ladder::Ladder(int x, int y, int is_up, std::string next_map, int which_coord) {
    this->sprite = new Sprite("sprites/ladder.bmp", 32, 48, 0);

    if (is_up) {
        this->sprite->set_frame(1);
    }

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 48.0f;

    this->x = (float)x;
    this->y = (float)y;

    this->which_coord = which_coord;

    this->next_map = next_map;
}

Ladder::~Ladder() {
    delete this->sprite;
}

void Ladder::step() {
    this->dst_rect.x = this->x - game->corner_x - 16.0f;
    this->dst_rect.y = this->y - game->corner_y - 38.0f;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 28);

    Mouse *mouse = closest_mouse(this->x, this->y, 8);

    if (mouse != nullptr) {
        game->map = this->next_map;
        save.puti(MOUSE_SPAWN_AT, this->which_coord);
    }
}
