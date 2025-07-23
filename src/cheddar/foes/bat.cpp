#include "bat.h"

#include "cheese.h"
#include "game.h"
#include "mouse.h"
#include "hurtbox.h"
#include "../items/toothpick.h"

#include <iostream>

FoeBat::FoeBat(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 125, 256.0f, 16.0f) {
    this->sprite = new Sprite("sprites/foe_bat.bmp", 24, 24, 100);

    this->dst_rect.w = 24.0f;
    this->dst_rect.h = 24.0f;

    game->push_object(HURTBOX_OBJ, HurtBoxFactory::Options(this->id, -8, -8, 16, 16));
}

FoeBat::~FoeBat() {
    delete this->sprite;
    this->sprite = nullptr;
}

void FoeBat::action(Mouse *mouse) {
    if (game->ticks - this->timer < 500) {
        return;
    }

    this->state = Foe::State::FORCE_RANDOM_TILE;
    this->timer = game->ticks;

    int x_off, y_off;
    HitBox::MakeOffset(x_dir, y_dir, &x_off, &y_off, 8.0f);
    HitBox::Properties props = {1, 500, 250};
    game->push_object(HITBOX_OBJ, HitBox::Options(this->id, this->id, x_off - 8, y_off - 8, 16, 16, props));
}

void FoeBat::attack_internal(int damage) {
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
#define ATTACK_ANIMATION 0 // 8
#define HURT_ANIMATION   0 // 16

void FoeBat::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::ACTION:
        this->state = Foe::State::FORCE_RANDOM_TILE;

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
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x, this->y - 16.0f, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->direction);
        this->sprite->interval_ms = 100;

        if (this->tile_choice == Foe::TileChoice::TOWARDS && this->current_distance <= 32.0f) {
            this->tile_choice = Foe::TileChoice::CIRCLE;
        } else if (this->tile_choice == Foe::TileChoice::CIRCLE && this->current_distance >= 80.0f) {
            this->tile_choice = Foe::TileChoice::TOWARDS;
        }

        break;
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x) - 12.0f;
    this->dst_rect.y = this->y - float(game->corner_y) - 12.0f;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 14);
}
