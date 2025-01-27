#include "enemy_bug.h"
#include "game.h"
#include "mouse.h"

#include <iostream>

EnemyBug::EnemyBug(const std::string &options) {
    int x = 0, y = 0;
    sscanf(options.c_str(), "%d,%d", &x, &y);

    this->x = float(x);
    this->y = float(y);
    this->target_x = this->x;
    this->target_y = this->y;

    this->sprite = new Sprite("sprites/bug.bmp", 16, 16, 100);

    this->dst_rect.w = 16.0f;
    this->dst_rect.h = 16.0f;

    // Enemy::Enemy configuration
    this->attack_distance = 16.0f;
}

EnemyBug::~EnemyBug() {
    delete this->sprite;
    this->sprite = nullptr;
}

void EnemyBug::step() {
    // move towards or mouse or random tile
    this->enemy_step();
    if (this->sprite->animation != this->animation) {
        this->sprite->set_animation(this->animation);
        if (this->animation > 7) {
            this->sprite->set_interval_ms(50);
        } else {
            this->sprite->set_interval_ms(100);
        }
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x);
    this->dst_rect.y = this->y - float(game->corner_y);
    game->push_sprite(this->sprite->texture, &this->sprite->frame, &this->dst_rect, 14);
}
