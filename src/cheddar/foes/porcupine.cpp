#include "porcupine.h"

#include "cheese.h"
#include "game.h"
#include "hurtbox.h"
#include "mouse.h"
#include "../items/toothpick.h"

#include <iostream>

FoePorcupine::FoePorcupine(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 225, 256.0f, 64.0f) {
    this->sprite = new Sprite("sprites/foe_porcupine.bmp", 16, 16, 100);

    this->dst_rect.w = 16.0f;
    this->dst_rect.h = 16.0f;

    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -8, -8, 16, 16));
}

FoePorcupine::~FoePorcupine() {
    delete this->sprite;
    this->sprite = nullptr;
}

void FoePorcupine::action(Mouse *mouse) {
    this->state = Foe::State::FORCE_RANDOM_TILE;
    this->tile_choice = Foe::TileChoice::AWAY;

    int spine_x_dir, spine_y_dir;
    dir_to_point(this->x, this->y, mouse->x, mouse->y, &spine_x_dir, &spine_y_dir);

    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d", (int)this->x, (int)this->y, spine_x_dir, spine_y_dir, this->id);
    game->push_object(ITEM_TOOTHPICK USE_OBJ, std::string(buff));
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

#define WALK_ANIMATION   0
#define ATTACK_ANIMATION 0 // 8
#define HURT_ANIMATION   0 // 16

void FoePorcupine::step() {
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

        game->push_health_bar(this->health, this->max_health, this->x, this->y - 12.0f, &this->icon_src, &this->icon_dst);

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
 
        if (this->tile_choice == Foe::TileChoice::AWAY && this->current_distance >= 128.0f) {
            this->tile_choice = Foe::TileChoice::TOWARDS;
        }

        break;
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x) - 8.0f;
    this->dst_rect.y = this->y - float(game->corner_y) - 8.0f;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 14);
}
