#include "mole.h"

#include "game.h"
#include "hurtbox.h"
#include "mouse.h"
#include "../items/cheese.h"
#include "../items/toothpick.h"

#include <iostream>

FoeMole::FoeMole(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 200, 512.0f, 32.0f) {
    this->sprite = new Sprite("sprites/foe_mole.bmp", 96, 64, 250);

    this->dst_rect.w = 96.0f;
    this->dst_rect.h = 64.0f;

    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -16, -16, 32, 32));
}

FoeMole::~FoeMole() {
    delete this->sprite;
    this->sprite = nullptr;
}

void FoeMole::action(Mouse *mouse) {
    this->state = Foe::State::ACTION;
    this->timer = game->ticks;

    HitBox::Properties props = {2, 500, 250};
    game->push_object(HITBOX_OBJ, HitBox::Options(this->id, this->id, -32, -32, 64, 64, props));
}

void FoeMole::attack_internal(int damage) {
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

#define WALK_ANIMATION   0
#define ATTACK_ANIMATION 0
#define HURT_ANIMATION   0

void FoeMole::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::ACTION:
        this->sprite->set_animation(ATTACK_ANIMATION + this->_displayed_direction);
        this->sprite->interval_ms = 100;
        this->sprite->update_frame();
        if (game->ticks - this->timer > 500) {
            this->state = Foe::State::IDLE;
        }

        break;

    case Foe::State::HURT:
        this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);
        if (game->ticks - this->timer > 500) {
            this->state = Foe::State::WALKING;
        }

        game->push_health_bar(this->health, this->max_health, this->x + this->off_x, this->y - 32.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    case Foe::State::DEAD:
        this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);
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

        game->push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x, this->y - 32.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);
        this->sprite->interval_ms = 250;
        this->sprite->update_frame();

        break;
    }

    this->dst_rect.x = this->x - float(game->corner_x) - 48.0f + this->off_x;
    this->dst_rect.y = this->y - float(game->corner_y) - 32.0f + this->off_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 56);
}
