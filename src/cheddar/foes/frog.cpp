#include "frog.h"
#include "game.h"
#include "../items/frog_tongue.h"

FoeFrog::FoeFrog(int x, int y, int spawner_id):
    Foe(float(x), float(y), spawner_id, 500, 128.0f, 32.0f) {
    this->sprite = new Sprite("sprites/foe_frog.bmp", 32, 32, 100);
    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;
    this->icon_offset = 16;
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
    game->push_object(std::string(ITEM_FROG_TONGUE USE_OBJ), std::string(options));
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
#define HURT_ANIMATION   8

void FoeFrog::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::ACTION:
        if (game->ticks - this->timer > 500) {
            this->state = Foe::State::FORCE_RANDOM_TILE;
        }

        break;

    case Foe::State::HURT:
        this->sprite->set_animation(HURT_ANIMATION + this->direction);
        if (game->ticks - this->timer > 500) {
            this->state = Foe::State::WALKING;
        }

        game->push_health_bar(this->health, this->max_health, this->x, this->y - 24.0f, &this->icon_src, &this->icon_dst);

        break;

    case Foe::State::DEAD:
        this->sprite->set_animation(HURT_ANIMATION + this->direction);
        if (game->ticks - this->timer > 2000) {
            // delete this object
            game->delete_object = true;

            char options[256];
            snprintf(options, sizeof(options), "%d,%d", int(this->x), int(this->y));
            game->push_object(std::string(ITEM_FROG_TONGUE DROPPED_OBJ), std::string(options));

            return;
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x, this->y - 24.0f, &this->icon_src, &this->icon_dst);

        break;

    default:
        this->sprite->set_animation(WALK_ANIMATION + this->direction);

        break;
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - float(game->corner_x) - 16.0f;
    this->dst_rect.y = this->y - float(game->corner_y) - 16.0f;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 24);
}
