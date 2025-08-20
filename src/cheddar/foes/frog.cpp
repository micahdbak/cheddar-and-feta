#include "frog.h"
#include "game.h"
#include "hurtbox.h"
#include "../items/cheese.h"
#include "../items/cannon_ball.h"

FoeFrog::FoeFrog(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 500, 128.0f, 64.0f) {
    this->sprite = new Sprite("sprites/foe_frog.bmp", 48, 48, 250);
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

    char options[256];
    snprintf(options, sizeof(options), "%d,%d,%d,%d,%d", (int)this->x, (int)this->y, this->x_dir, this->y_dir, this->id);
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
    case Foe::State::ACTION:
        this->sprite->set_animation(ACTION_ANIMATION + this->_displayed_direction);
        if (game->ticks - this->timer > 750) {
            this->state = Foe::State::FORCE_RANDOM_TILE;
        }

        break;

    case Foe::State::HURT:
        this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);
        if (game->ticks - this->timer > 250) {
            this->state = Foe::State::WALKING;
        }

        game->push_health_bar(this->health, this->max_health, this->x + this->off_x, this->y - 24.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    case Foe::State::DEAD:
        this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);
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

            char options[256];
            snprintf(options, sizeof(options), "%d,%d", int(this->x), int(this->y));
            game->push_object(std::string(ITEM_CANNON_BALL DROPPED_OBJ), std::string(options));

            return;
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x, this->y - 24.0f + this->off_y, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);

        break;
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x) - 24.0f + this->off_x;
    this->dst_rect.y = this->y - float(game->corner_y) - 24.0f + this->off_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 32);
}
