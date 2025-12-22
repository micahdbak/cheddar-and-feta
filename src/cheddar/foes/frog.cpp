#include "frog.h"
#include "game.h"
#include "hurtbox.h"
#include "save_data.h"
#include "audio_playback.h"
#include "../items/cheese.h"
#include "../items/cannon_ball.h"

FoeFrog::FoeFrog(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 500, 128.0f, 64.0f) {
    this->sprite = new Sprite("sprites/foe_frog.bmp", 48, 48, 125);
    this->dst_rect.w = 48.0f;
    this->dst_rect.h = 48.0f;
    this->icon_offset = 16;

    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -16, -16, 32, 32));
}

FoeFrog::~FoeFrog() {
    delete this->sprite;
    this->sprite = nullptr;
}

void FoeFrog::action(Mouse *mouse) {
    if (game->ticks - this->attack_timer < 2000) {
        this->state = Foe::State::FORCE_RANDOM_TILE;
        return;
    }

    this->attack_timer = game->ticks;

    this->state = Foe::State::ACTION;
    this->timer = game->ticks;

    play_audio("sfx/cannon.wav", 1.0, this->x, this->y);

    char options[256];
    snprintf(options, sizeof(options), "%d,%d,%d,%d,%d", (int)(this->x + this->off_x), (int)(this->y + this->off_y), this->x_dir, this->y_dir, this->id);
    game->push_object(std::string(ITEM_CANNON_BALL USE_OBJ), std::string(options));

    // for action animation
    this->sprite->set_frame(0);
}

void FoeFrog::attack_internal(int damage) {
    this->health -= damage;

    if (this->health <= 0) {
        this->health = 0;
        this->state = Foe::State::DEAD;
        this->remove_from_foes();

        int kills = save.geti(FOE_FROG_OBJ STATS) + 1;
        save.puti(FOE_FROG_OBJ STATS, kills);
    } else {
        this->state = Foe::State::HURT;
    }

    this->timer = game->ticks;
}

#define WALK_ANIMATION   0
#define ACTION_ANIMATION 8

void FoeFrog::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::IDLE:
        if (this->prev_state != this->state) {
            switch (SDL_rand(2)) {
            case 0: play_audio("sfx/tank_roll1.wav", 0.5, this->x, this->y); break;
            case 1: play_audio("sfx/tank_roll2.wav", 0.5, this->x, this->y); break;
            }
        }

        break;

    case Foe::State::ACTION:
        this->sprite->set_animation(ACTION_ANIMATION + this->_displayed_direction);
        if (game->ticks - this->timer > 500) {
            this->state = Foe::State::FORCE_RANDOM_TILE;
        }

        break;

    case Foe::State::HURT:
        if (this->prev_state != this->state) {
            play_audio("sfx/tank_hurt.wav", 1.0, this->x, this->y);
        }

        this->sprite->set_animation(ACTION_ANIMATION + this->_displayed_direction);
        this->sprite->set_frame(0);
        if (game->ticks - this->timer > 250) {
            this->state = Foe::State::WALKING;
        }

        game->push_health_bar(this->health, this->max_health, this->x + this->off_x, this->y - 24.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    case Foe::State::DEAD:
        if (this->prev_state != this->state) {
            play_audio("sfx/tank_die.wav", 1.0, this->x, this->y);
        }

        this->sprite->set_animation(ACTION_ANIMATION + this->_displayed_direction);
        this->sprite->set_frame(0);
        if (game->ticks - this->timer > 2000) {
            // delete this object
            game->delete_object = true;
            Cheese::drop_cheese(this->x, this->y, 3, 5);

            char options[256];
            snprintf(options, sizeof(options), "%d,%d", int(this->x + this->off_x), int(this->y + this->off_y));
            game->push_object(std::string(ITEM_CANNON_BALL DROPPED_OBJ), std::string(options));

            return;
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x, this->y - 24.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);

        break;
    }

    this->prev_state = this->state;

    this->sprite->update_frame();
    this->dst_rect.x = this->x - 24.0f + this->off_x;
    this->dst_rect.y = this->y - 24.0f + this->off_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 32);
}
