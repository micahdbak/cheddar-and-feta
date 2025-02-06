#include "cheese.h"
#include "enemy_bug.h"
#include "game.h"
#include "mouse.h"

#include <iostream>

EnemyBug::EnemyBug(int x, int y) {
    this->x = float(x);
    this->y = float(y);
    this->target_x = this->x;
    this->target_y = this->y;

    this->sprite = new Sprite("sprites/bug.bmp", 16, 16, 100);

    this->dst_rect.w = 16.0f;
    this->dst_rect.h = 16.0f;

    // Enemy::Enemy configuration
    this->sight_distance = 64.0f;
    this->attack_distance = 24.0f;
    this->speed = 16.0f;
    this->max_health = this->health = 2;
}

EnemyBug::~EnemyBug() {
    delete this->sprite;
    this->sprite = nullptr;
}

void EnemyBug::step() {
    if (this->dead) {
        if (this->dead_ticks == 0) {
            this->dead_ticks = game->ticks;
        } else if (game->ticks - this->dead_ticks > 1000) {
            // delete this object
            game->delete_object = true;
            game->push_object(ENEMY_BUG_OBJ, "256,256");

            int cheese_amount = SDL_rand(8); // 0..7

            // add cheese for the player to pick up
            if (cheese_amount > 4) { // 5..7
                cheese_amount -= 4; // 1..3
                char cheese_opt[256];
                snprintf(cheese_opt, sizeof(cheese_opt), "%d,%d,%d", int(this->x) + game->tile_width/2, int(this->y) + game->tile_height/2, cheese_amount);
                game->push_object(CHEESE_OBJ, std::string(cheese_opt));
            }
            return;
        }
    }
    
    // move towards or mouse or random tile
    this->enemy_step();
    if (this->sprite->animation != this->animation) {
        this->sprite->set_animation(this->animation);
        if (this->animation > 7) {
            this->sprite->interval_ms = 50;
        } else {
            this->sprite->interval_ms = 100;
        }
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x);
    this->dst_rect.y = this->y - float(game->corner_y);
    game->push_sprite(this->sprite->texture, &this->sprite->frame, &this->dst_rect, 14);
}
