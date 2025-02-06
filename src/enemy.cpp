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
    if (this->dead) {
        game->draw_rect(&this->draw_rect, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        return;
    }

    // remove this enemy from the enemies list
    for (auto it = enemies.begin(); it != enemies.end(); it++) {
        if (*it == this) {
            enemies.erase(it);
            break;
        }
    }
}

void Enemy::enemy_step() {
    if (this->dead) {
        game->draw_rect(&this->draw_rect, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        this->_draw_skull_and_bones();
        return;
    }

    int dx = sign(this->target_x - int(this->x));
    int dy = sign(this->target_y - int(this->y));

    if (this->animation > 15) {
        game->draw_rect(&this->draw_rect, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

        if (game->ticks - this->attack_ticks > 500) {
            this->animation = this->animation % 8;
        } else this->_draw_health_bar();
    } else if (dx == 0 && dy == 0) {
        this->x = this->target_x;
        this->y = this->target_y;

        // coordinate of center of tile
        float center_x = this->x + float(game->tile_width/2);
        float center_y = this->y + float(game->tile_height/2);

        float distance = distance_between_points(center_x, center_y, mouse->x, mouse->y);

        bool in_sight = false;
        int attack = 0;

        if (distance < this->attack_distance) {
            if (mouse->attack(this->damage)) {
                attack = 8;
                dir_to_point(center_x, center_y, mouse->x, mouse->y, &mouse->throw_x, &mouse->throw_y);
            }
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

bool Enemy::attack(int damage) {
    if (this->animation > 15) return false;

    this->animation = (this->animation % 8) + 16;
    this->attack_ticks = game->ticks;

    damage -= this->armour;
    if (damage > 0)
        this->health -= damage;

    if (this->health <= 0) {
        this->health = 0; // let's not go negative
        this->dead = true;

        // remove this enemy from the enemies list
        for (auto it = enemies.begin(); it != enemies.end(); it++) {
            if (*it == this) {
                enemies.erase(it);
                break;
            }
        }

        return true;
    }

    this->_draw_health_bar();

    return true;
}

void Enemy::_draw_health_bar() {
    int w_mul = 2;
    if (max_health < 4) {
        w_mul = 3;
    }

    int health_w = (this->max_health*w_mul) + 2;
    int health_x = int(this->x) + (game->tile_width/2) - (health_w/2) - game->corner_x;
    int health_y = int(this->y) + game->tile_height - this->height - 9 - game->corner_y;
    this->draw_rect = { float(health_x), float(health_y), float(health_w), 5.0f };
    game->draw_rect(&this->draw_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
    this->draw_rect.w = float((this->health*w_mul) + 1);
    game->draw_rect(&this->draw_rect, 255, 128, 96, 255, SDL_BLENDMODE_NONE);
    this->draw_rect.w = float(health_w);
    game->draw_outline(&this->draw_rect, 255, 255, 255, 255, SDL_BLENDMODE_NONE);
}

void Enemy::_draw_skull_and_bones() {
    int icon_x = int(this->x) + (game->tile_width/2) - (ICON_SIZE/2) - game->corner_x;
    int icon_y = int(this->y) + game->tile_height - ICON_SIZE - this->height - 2 - game->corner_y;
    this->draw_rect = { float(icon_x), float(icon_y), float(ICON_SIZE), float(ICON_SIZE) };
    game->draw_icon(SKULL_AND_BONES_ICON, &this->draw_rect);
}
