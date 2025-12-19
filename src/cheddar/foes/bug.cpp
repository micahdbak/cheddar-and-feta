#include "bug.h"

#include "game.h"
#include "hurtbox.h"
#include "mouse.h"
#include "save_data.h"
#include "audio_playback.h"
#include "../items/cheese.h"
#include "../items/toothpick.h"

#include <iostream>

FoeBug::FoeBug(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, spawner_id == -2 ? 1000 : 500, 256.0f, 16.0f) {
    this->sprite = new Sprite("sprites/foe_bug.bmp", 32, 32, 100);

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;

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

        int kills = save.geti(FOE_BUG_OBJ STATS) + 1;
        save.puti(FOE_BUG_OBJ STATS, kills);
    } else {
        this->state = Foe::State::HURT;
    }

    this->timer = game->ticks;
}

#define WALK_ANIMATION   0
#define ATTACK_ANIMATION 0
#define HURT_ANIMATION   8

void FoeBug::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::IDLE:
        if (this->prev_state != this->state) {
            switch (SDL_rand(4)) {
            case 0: play_audio("sfx/ant_walk1.wav", 0.5, this->x, this->y); break;
            case 1: play_audio("sfx/ant_walk2.wav", 0.5, this->x, this->y); break;
            case 2: play_audio("sfx/ant_walk3.wav", 0.5, this->x, this->y); break;
            default: break; // no sound
            }
        }

        break;

    case Foe::State::ACTION:
        if (this->prev_state != this->state) {
            play_audio("sfx/ant_attack.wav", 1.0, this->x, this->y);
        }

        this->sprite->set_animation(ATTACK_ANIMATION + this->_displayed_direction);
        this->sprite->interval_ms = 50;
        if (game->ticks - this->timer > 1000) {
            this->state = Foe::State::FORCE_RANDOM_TILE;
        }

        break;

    case Foe::State::HURT:
        if (this->prev_state != this->state) {
            play_audio("sfx/ant_hurt.wav", 1.0, this->x, this->y);
        }

        this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);
        if (game->ticks - this->timer > 500) {
            this->state = Foe::State::WALKING;
        }

        game->push_health_bar(this->health, this->max_health, this->x + this->off_x, this->y - 16.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    case Foe::State::DEAD:
        if (this->prev_state != this->state) {
            play_audio("sfx/ant_die.wav", 1.0, this->x, this->y);
        }

        this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);
        if (game->ticks - this->timer > 2000) {
            // delete this object
            game->delete_object = true;
            Cheese::drop_cheese(this->x, this->y, 1, 1);

            return;
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x, this->y - 14.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);
        this->sprite->interval_ms = 100;

        break;
    }

    this->prev_state = this->state;

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x) - 16.0f + this->off_x;
    this->dst_rect.y = this->y - float(game->corner_y) - 16.0f + this->off_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 18);
}
