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

    if (mice_locked)
        return;

    int dx = cnf_sign(this->target_x - int(this->x));
    int dy = cnf_sign(this->target_y - int(this->y));

    if (this->animation > 15) {
        game->draw_rect(&this->draw_rect, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

        if (game->ticks - this->attack_ticks > 500) {
            this->animation = this->animation % 8;
        } else {
            this->draw_rect = game->draw_health_bar(
                this->health, this->max_health,
                int(this->x) + (game->tile_width/2) - game->corner_x,
                int(this->y) + game->tile_height - this->height - 4 - game->corner_y
            );
        }
    } else if (dx == 0 && dy == 0) {
        this->x = this->target_x;
        this->y = this->target_y;

        // coordinate of center of tile
        float center_x = this->x + float(game->tile_width/2);
        float center_y = this->y + float(game->tile_height/2);

        float distance_cheddar = distance_between_points(center_x, center_y, cheddar->x, cheddar->y);
        float distance_feta = distance_between_points(center_x, center_y, feta->x, feta->y);
        bool cheddar_closer = distance_cheddar < distance_feta;

        bool in_sight = false;
        int attack = 0;

        if (cheddar_closer) {
            if (distance_cheddar < this->attack_distance) {
                if (cheddar->attack(this->damage)) {
                    attack = 8;
                    dir_to_point(center_x, center_y, cheddar->x, cheddar->y, &cheddar->throw_x, &cheddar->throw_y);
                }
            } else if (distance_cheddar < this->sight_distance) {
                if (game->in_sight(int(this->x), int(this->y), int(cheddar->x), int(cheddar->y), &this->target_x, &this->target_y)) {
                    in_sight = true;
                } else if (distance_feta < this->sight_distance) {
                    // if Cheddar isn't in sight, but Feta is within sight distance, see if they're in sight
                    in_sight = game->in_sight(int(this->x), int(this->y), int(feta->x), int(feta->y), &this->target_x, &this->target_y);
                }
            }
        } else {
            if (distance_feta < this->attack_distance) {
                if (feta->attack(this->damage)) {
                    attack = 8;
                    dir_to_point(center_x, center_y, feta->x, feta->y, &feta->throw_x, &feta->throw_y);
                }
            } else if (distance_feta < this->sight_distance) {
                if (game->in_sight(int(this->x), int(this->y), int(feta->x), int(feta->y), &this->target_x, &this->target_y)) {
                    in_sight = true;
                } else if (distance_cheddar < this->sight_distance) {
                    // if Feta isn't in sight, but Cheddar is within sight distance, see if they're in sight
                    in_sight = game->in_sight(int(this->x), int(this->y), int(cheddar->x), int(cheddar->y), &this->target_x, &this->target_y);
                }
            }
        }

        // if either mouse isn't in sight, let's move to a random tile
        if (!in_sight)
            game->random_target(int(this->x), int(this->y), &this->target_x, &this->target_y);

        this->_speed = (in_sight ? this->speed : (this->speed / 2.0f)) + (4.0f * SDL_randf());

        dx = cnf_sign(this->target_x - int(this->x));
        dy = cnf_sign(this->target_y - int(this->y));
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

    this->draw_rect = game->draw_health_bar(
        this->health, this->max_health,
        int(this->x) + (game->tile_width/2) - game->corner_x,
        int(this->y) + game->tile_height - this->height - 4 - game->corner_y
    );

    return true;
}

void Enemy::_draw_skull_and_bones() {
    int icon_x = int(this->x) + (game->tile_width/2) - 8 - game->corner_x;
    int icon_y = int(this->y) + game->tile_height - 16 - this->height - 2 - game->corner_y;
    this->draw_rect = { float(icon_x), float(icon_y), 16.0f, 16.0f };
    game->draw_icon(SKULL_AND_BONES_ICON, &this->draw_rect);
}
