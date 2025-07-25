#include "bug.h"

#include "cheese.h"
#include "game.h"
#include "hurtbox.h"
#include "mouse.h"
#include "../items/toothpick.h"

#include <iostream>

FoeBug::FoeBug(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 500, 256.0f, 16.0f) {
    this->sprite = new Sprite("sprites/foe_bug.bmp", 18, 18, 100);

    this->dst_rect.w = 18.0f;
    this->dst_rect.h = 18.0f;

    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -8, -8, 16, 16));
}

FoeBug::~FoeBug() {
    delete this->sprite;
    this->sprite = nullptr;
}

void FoeBug::action(Mouse *mouse) {
    this->state = Foe::State::ACTION;
    this->timer = game->ticks;

    int x_off, y_off;
    HitBox::MakeOffset(x_dir, y_dir, &x_off, &y_off, 8.0f);
    HitBox::Properties props = {1, 500, 250};
    game->push_object(HITBOX_OBJ, HitBox::Options(this->id, this->id, x_off - 8, y_off - 8, 16, 16, props));
}

void FoeBug::attack_internal(int damage) {
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
#define ATTACK_ANIMATION 8
#define HURT_ANIMATION   16

void FoeBug::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::ACTION:
        this->sprite->set_animation(ATTACK_ANIMATION + this->direction);
        this->sprite->interval_ms = 50;
        if (game->ticks - this->timer > 1000) {
            this->state = Foe::State::IDLE;
        }

        break;

    case Foe::State::HURT:
        this->sprite->set_animation(HURT_ANIMATION + this->direction);
        if (game->ticks - this->timer > 1000) {
            this->state = Foe::State::WALKING;
        }

        game->push_health_bar(this->health, this->max_health, this->x, this->y - 16.0f, &this->icon_src, &this->icon_dst);

        break;

    case Foe::State::DEAD:
        this->sprite->set_animation(HURT_ANIMATION + this->direction);
        if (game->ticks - this->timer > 2000) {
            // delete this object
            game->delete_object = true;
            return;

            int cheese_amount = SDL_rand(8); // 0..7

            // add cheese for the player to pick up
            if (cheese_amount > 4) { // 5..7
                cheese_amount -= 4; // 1..3
                char cheese_opt[256];
                snprintf(cheese_opt, sizeof(cheese_opt), "%d,%d,%d", int(this->x) + game->tile_width/2, int(this->y) + game->tile_height/2, cheese_amount);
                game->push_object(CHEESE_OBJ, std::string(cheese_opt));
            }
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x, this->y - 14.0f, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->direction);
        this->sprite->interval_ms = 100;

        break;
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x) - 9.0f;
    this->dst_rect.y = this->y - float(game->corner_y) - 9.0f;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 14);
}
