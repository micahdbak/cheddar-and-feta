#include "item_toothpick.h"
#include "item.h"

#include <iostream>

ThrownToothpick::ThrownToothpick(float x, float y, int x_dir, int y_dir)
    : ThrownItem(x, y, x_dir, y_dir) {
    this->sprite = new Sprite("sprites/toothpick.bmp", 16, 16, 0);
    this->sprite->set_animation(direction_from_dirs(x_dir, y_dir));
    this->dst_rect.w = 16.0f;
    this->dst_rect.h = 16.0f;
}

ThrownToothpick::~ThrownToothpick() {
    delete this->sprite;
    this->sprite = nullptr;
}

void ThrownToothpick::step() {
    this->thrown_step();

    this->dst_rect.x = float(this->x - game->corner_x - 8);
    this->dst_rect.y = float(this->y - game->corner_y - 8);

    game->push_sprite(this->sprite->texture, &this->sprite->frame, &this->dst_rect, 16);
}

void ThrownToothpick::on_hit(Enemy *enemy) {
    enemy->attack(1);
}

void ThrownToothpick::on_miss() {
    char options[256];
    snprintf(options, sizeof(options), "%d,%d", int(this->x), int(this->y));
    game->push_object(ITEM_TOOTHPICK DROPPED_OBJ, std::string(options));
}
