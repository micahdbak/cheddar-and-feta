#include "porcupine.h"

#include "game.h"
#include "hurtbox.h"
#include "mouse.h"
#include "../items/cheese.h"
#include "../items/toothpick.h"

#include <iostream>

FoePorcupine::FoePorcupine(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 250, 256.0f, 64.0f) {
    this->sprite = new Sprite("sprites/foe_porcupine.bmp", 32, 32, 250);

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;

    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -6, -8, 12, 16));
}

FoePorcupine::~FoePorcupine() {
    delete this->sprite;
    this->sprite = nullptr;
}

#define WALK_ANIMATION   0
#define ATTACK_ANIMATION 8
#define DEAD_ANIMATION   16

void FoePorcupine::action(Mouse *mouse) {
    dir_to_point(this->x, this->y, mouse->x, mouse->y, &this->spine_x_dir, &this->spine_y_dir);
    this->sprite->set_animation(ATTACK_ANIMATION + direction_from_dirs(this->spine_x_dir, this->spine_y_dir));
    this->sprite->set_frame(0);
    this->timer = game->ticks;
}

void FoePorcupine::attack_internal(int damage) {
    this->health -= damage;

    if (this->health <= 0) {
        this->health = 0;
        this->state = Foe::State::DEAD;
        this->remove_from_foes();
    } else {
        this->state = Foe::State::HURT;
    }

    this->timer = game->ticks;
}

void FoePorcupine::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::ACTION:
        this->sprite->interval_ms = 200;
        if (game->ticks - this->timer > 600) {
            this->state = Foe::State::WALKING;
            this->tile_choice = Foe::TileChoice::AWAY;
            this->sprite->set_animation(WALK_ANIMATION + this->direction);
        } else if (game->ticks - this->timer > 400 && (this->spine_x_dir != 0 || this->spine_y_dir != 0)) {
            char buff[256];
            snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d", (int)(this->x + this->off_x), (int)(this->y + this->off_y), this->spine_x_dir, this->spine_y_dir, this->id);
            game->push_object(ITEM_TOOTHPICK USE_OBJ, std::string(buff));
            this->spine_x_dir = this->spine_y_dir = 0;
        }

        break;

    case Foe::State::HURT:
        if (game->ticks - this->timer > 250) {
            this->state = Foe::State::WALKING;
        }

        game->push_health_bar(this->health, this->max_health, this->x + this->off_x, this->y - 12.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    case Foe::State::DEAD:
        this->sprite->set_animation(DEAD_ANIMATION + this->direction);
        if (game->ticks - this->timer > 2000) {
            // delete this object
            game->delete_object = true;

            int cheese_amount = SDL_rand(8); // 0..7

            // add cheese for the player to pick up
            if (cheese_amount > 4) { // 5..7
                cheese_amount -= 4; // 1..3
                char cheese_opt[256];
                game->push_object(ITEM_CHEESE DROPPED_OBJ, Cheese::Options(this->x, this->y, cheese_amount));
            }

            return;
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x, this->y - 16.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->direction);
        this->sprite->interval_ms = 100;
 
        if (this->tile_choice == Foe::TileChoice::AWAY && this->current_distance >= 48.0f) {
            this->tile_choice = Foe::TileChoice::TOWARDS;
        }

        break;
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x) - 16.0f + this->off_x;
    this->dst_rect.y = this->y - float(game->corner_y) - 16.0f + this->off_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
}
