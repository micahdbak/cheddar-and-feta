#include "game.h"
#include "save_station.h"

SaveStation::SaveStation(int x, int y): x(x), y(y) {
    this->sprite = new Sprite("sprites/save_station.bmp", 32, 24, -1);
    this->dst_rect = { float(x), float(y), 32.0f, 24.0f };
}

SaveStation::~SaveStation() {
    delete this->sprite;
    this->sprite = nullptr;
}

void SaveStation::step() {
    this->dst_rect.x = float(this->x - game->corner_x);
    this->dst_rect.y = float(this->y - game->corner_y);

    game->push_sprite(this->sprite->texture, NULL, &this->dst_rect, 18);
}
