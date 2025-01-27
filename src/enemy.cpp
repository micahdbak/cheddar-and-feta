#include "enemy.h"
#include "game.h"
#include "mouse.h"

#include <iostream>

std::vector<Enemy *> enemies;
int sel_enemy = 0;

Enemy::Enemy() {
    enemies.push_back(this);
}

Enemy::~Enemy() {
    // remove this enemy from the enemies list
    for (auto it = enemies.begin(); it != enemies.end(); it++) {
        if (*it == this) {
            enemies.erase(it);
            break;
        }
    }
}

void Enemy::enemy_step() {
    int dx = sign(this->target_x - int(this->x));
    int dy = sign(this->target_y - int(this->y));

    if (this->animation > 15) {
        if (game->ticks - this->attack_ticks > 500) {
            this->animation = this->animation % 8;
        } // else, wait, stay attacked
    } else if (dx == 0 && dy == 0) {
        this->x = this->target_x;
        this->y = this->target_y;

        float distance = distance_between_points(this->x + game->tile_width/2, this->y + game->tile_height/2, mouse->x, mouse->y);

        bool in_sight = false;
        int attack = 0;

        if (distance < this->attack_distance) {
            attack = mouse->attack() ? 8 : 0;
        } else if (distance < this->sight_distance)
            in_sight = game->in_sight(int(this->x), int(this->y), int(mouse->x), int(mouse->y), &this->target_x, &this->target_y);

        // if player isn't in sight, let's move to a random tile
        if (!in_sight)
            game->random_target(int(this->x), int(this->y), &this->target_x, &this->target_y);

        this->_speed = (in_sight ? this->speed : (this->speed / 2.0f)) + (4.0f * SDL_randf());

        dx = sign(this->target_x - int(this->x));
        dy = sign(this->target_y - int(this->y));
        if (dx > 0) {
            this->animation = 2 - dy + attack;
        } else if (dx < 0) {
            this->animation = 6 + dy + attack;
        } else if (dy > 0) {
            this->animation = 0 + attack;
        } else if (dy < 0) {
            this->animation = 4 + attack;
        }
    } else {
        this->x += float(dx) * (dy != 0 ? DIAG_MULTIPLIER : 1.0f) * this->_speed * game->delta;
        this->y += float(dy) * (dx != 0 ? DIAG_MULTIPLIER : 1.0f) * this->_speed * game->delta;
    }
}

bool Enemy::attack() {
    if (this->animation > 15) return false;

    this->animation = (this->animation % 8) + 16;
    this->attack_ticks = game->ticks;

    return true;
}
