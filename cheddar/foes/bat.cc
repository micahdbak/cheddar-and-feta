#include "bat.h"

#include "game.h"
#include "mouse.h"
#include "hurtbox.h"
#include "save_data.h"
#include "audio_playback.h"
#include "../items/cheese.h"
#include "../items/toothpick.h"

#include <iostream>
#include <cmath>

#define CIRCLE_TICKS 2000

FoeBat::FoeBat(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 175, 100, 256.0f, 16.0f) {
    this->sprite = new Sprite("sprites/foe_bat.bmp", 32, 32, 45); // approx every 3 frames

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;

    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -8, -8, 16, 16));

    this->base_ticks_offset = SDL_rand(CIRCLE_TICKS + 1);
    this->ticks_offset = base_ticks_offset;
}

FoeBat::~FoeBat() {
    delete this->sprite;
    this->sprite = nullptr;
}

void FoeBat::action(Mouse *mouse) {
    if (game->ticks - this->timer < 500) {
        this->state = Foe::State::FORCE_RANDOM_TILE;
        return;
    }

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

        int kills = save.geti(FOE_BAT_OBJ STATS) + 1;
        save.puti(FOE_BAT_OBJ STATS, kills);
    } else {
        this->state = Foe::State::THROW_AWAY_FROM;
        this->hurt_timer = game->ticks;
    }

    this->timer = game->ticks;
}

#define WALK_ANIMATION   0
#define HURT_ANIMATION   8 // 16

void FoeBat::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::IDLE:
        if (this->prev_state != this->state && game->ticks - this->audio_timer > 500) {
            this->audio_timer = game->ticks;
            play_audio("sfx/ant_fly.wav", 0.5, this->x, this->y, false);
        }

        break;

    case Foe::State::ACTION:
        if (this->prev_state != this->state) {
            play_audio("sfx/ant_attack.wav", 1.0, this->x, this->y, false);
        }
        
        if (game->ticks - this->timer > 250) {
            this->state = State::FORCE_RANDOM_TILE;
        }

        break;

    case Foe::State::THROWN:
        if (this->prev_state != this->state) {
            play_audio("sfx/ant_hurt.wav", 1.0, this->x, this->y, false);
        }

        break;

    case Foe::State::DEAD:
        if (this->prev_state != this->state) {
            play_audio("sfx/ant_die.wav", 1.0, this->x, this->y, false);
        }

        this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);

        if (game->ticks - this->timer > 2000) {
            // delete this object
            game->delete_object = true;
            Cheese::drop_cheese(this->x, this->y, 1, 3);

            return;
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x, this->y - 16.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);

        if (this->tile_choice == Foe::TileChoice::TOWARDS && this->current_distance <= 32.0f) {
            this->tile_choice = Foe::TileChoice::CIRCLE;
        } else if (this->tile_choice == Foe::TileChoice::CIRCLE && this->current_distance >= 128.0f) {
            this->tile_choice = Foe::TileChoice::TOWARDS;
        }

        break;
    }

    if (game->ticks - this->hurt_timer < 333 && this->state != Foe::State::DEAD) {
        this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);
        game->push_health_bar(this->health, this->max_health, this->x + this->off_x, this->y - 16.0f + this->off_y, &this->icon_src, &this->icon_dst);
    }

    if (this->state != Foe::State::HURT && this->state != Foe::State::DEAD) {
        float angle = (float)((game->ticks - this->ticks_offset) % CIRCLE_TICKS) / 1000.0f;
        angle *= M_PI;
        this->off_x = sinf(angle) * 8.0f;
        this->off_y = cosf(angle) * 8.0f;
    } else {
        this->ticks_offset = game->ticks - this->timer + base_ticks_offset;
    }

    this->prev_state = this->state;

    this->sprite->update_frame();
    this->dst_rect.x = this->x - 16.0f + this->off_x;
    this->dst_rect.y = this->y - 16.0f + this->off_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 26);
}
