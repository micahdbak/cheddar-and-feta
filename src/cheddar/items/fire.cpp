#include "fire.h"
#include "SDL3/SDL_stdinc.h"

static int fire_counter = 0;

Fire::Fire(float x, float y, int x_dir, int y_dir):
    TrackingItem(x, y, 24.0f, -1), x_dir(x_dir), y_dir(y_dir) {
    this->sprite = new Sprite("sprites/fire.bmp", 16, 16, 100);
    this->dst_rect.w = this->sprite->frame_w;
    this->dst_rect.h = this->sprite->frame_h;
    this->timer = game->ticks;

    fire_counter++;
    this->sprite->set_animation(fire_counter % 2);
    this->sprite->set_frame(SDL_rand(4));

    this->mov_speed = SDL_randf() * 64.0f + 120.0f;

    if (!x_dir && !y_dir) {
        this->state = Fire::State::STATIONARY;
    }
}

Fire::~Fire() {
    delete this->sprite;
}

void Fire::on_mouse(Mouse *mouse) {
    if (game->ticks - this->attack_timer > 500) {
        mouse->attack(1);
        this->attack_timer = game->ticks;
    }
}

void Fire::on_foe(Foe *foe) {
    if (game->ticks - this->attack_timer > 500) {
        foe->attack(1);
        this->attack_timer = game->ticks;
    }
}

void Fire::step() {
    this->tracking_step();

    if (this->state == Fire::State::MOVING) {
        float dx = float(this->x_dir) * (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;
        float dy = float(this->y_dir) * (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;
        float new_x = this->x + dx;
        float new_y = this->y + dy;

        if (!game->point_in_collider(new_x, this->y)) {
            this->x = new_x;
        }

        if (!game->point_in_collider(this->x, new_y)) {
            this->y = new_y;
        }

        if (game->ticks - this->timer > 100) {
            this->state = Fire::State::STATIONARY;
        }
    }

    if (game->ticks - this->timer > 5000) {
        game->delete_object = true;
        return;
    } else if (game->ticks - this->timer > 4875) {
        this->sprite->set_animation(3);
    } else if (game->ticks - this->timer > 4750) {
        this->sprite->set_animation(2);
        this->sprite->interval_ms = 1000;
    }

    this->sprite->update_frame();

    this->dst_rect.x = this->x - (float)(int)(this->sprite->frame_w/2) - game->corner_x;
    this->dst_rect.y = this->y - (float)(int)(this->sprite->frame_h/2) - game->corner_y;

    game->push_sprite("sprites/fire.bmp", this->sprite->texture, &this->sprite->frame, &this->dst_rect, 12);
}
