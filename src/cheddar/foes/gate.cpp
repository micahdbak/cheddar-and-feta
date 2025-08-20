#include "gate.h"

#include "game.h"
#include "spawner.h"

#include <iostream>

FoeGate::FoeGate(int x, int y, int spawner_id, int animation):
    spawner_id(spawner_id) {
    this->sprite = new Sprite("sprites/gate.bmp", 48, 48, 0);
    this->dst_rect.w = 48.0f;
    this->dst_rect.h = 48.0f;
    this->sprite->set_animation(animation);

    x = x - (x % game->tile_width);
    y = y - (y % game->tile_height);
    this->x = float(x);
    this->y = float(y);

    /* gates are 48x48; tiles are 16x16; therefore a gate should look like:
     * @ - + - + - + <- @ is gate coordinate
     * | O | O | O |
     * + - + - + - +
     * | O | O | O |
     * + - + - + - +
     * | # | # | # | <- # is a collider
     */

    this->colliders_offset = ((y/game->tile_height) + 1) * game->cols;
    this->colliders_x = x / game->tile_width;
    for (int i = 0; i < 3; i++) {
        int coord = this->colliders_offset + this->colliders_x + i;
        this->colliders.push_back(game->collision[coord]);
        game->collision[coord] = 0; // solid collider
    }
}

FoeGate::~FoeGate() {
    delete this->sprite;
}

void FoeGate::step() {
    if (!this->opened && spawners[this->spawner_id] != nullptr && spawners[this->spawner_id]->empty) {
        // remove colliders
        for (int i = 0; i < 3; i++) {
            int coord = this->colliders_offset + this->colliders_x + i;
            game->collision[coord] = this->colliders[i];
        }
        this->colliders.clear();

        this->sprite->set_frame(1); // opened frame
        this->opened = true;
    }

    this->dst_rect.x = this->x - game->corner_x;
    this->dst_rect.y = this->y - game->corner_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 24);
}
