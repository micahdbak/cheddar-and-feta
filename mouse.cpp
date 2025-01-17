#include "game.h"
#include "keyboard.h"
#include "mouse.h"

Mouse::Mouse(int x, int y) {
    this->sprite = new Sprite("sprites/mouse.bmp", 24, 24, 250);
    this->texture = this->sprite->texture;
    this->src_rect = &this->sprite->frame;

    this->dst_rect.x = float(x);
    this->dst_rect.y = float(y);
    this->dst_rect.w = 24.0f;
    this->dst_rect.h = 24.0f;
}

Mouse::~Mouse() {
    delete this->sprite;
    this->sprite = nullptr;
}

void Mouse::step() {
    float mov_speed;
    if (keyboard.is_down(SDLK_LSHIFT)) {
        mov_speed = 48.0f;
        this->sprite->set_interval_ms(100);
    } else {
        mov_speed = 24.0f;
        this->sprite->set_interval_ms(250);
    }

    int x_dir = int(keyboard.is_down(SDLK_RIGHT)) - int(keyboard.is_down(SDLK_LEFT));
    int y_dir = int(keyboard.is_down(SDLK_DOWN)) - int(keyboard.is_down(SDLK_UP));
    this->dst_rect.x += float(x_dir) * (y_dir != 0 ? 0.7071f : 1.0f) * mov_speed * game->delta;
    this->dst_rect.y += float(y_dir) * (x_dir != 0 ? 0.7071f : 1.0f) * mov_speed * game->delta;

    if (x_dir != 0 || y_dir != 0) {
        this->sprite->update_frame();
    } else {
        this->sprite->set_frame(0);
    }

    if (x_dir > 0) {
        this->sprite->set_animation(2 - y_dir);
    } else if (x_dir < 0) {
        this->sprite->set_animation(6 + y_dir);
    } else if (y_dir > 0) {
        this->sprite->set_animation(0);
    } else if (y_dir < 0) {
        this->sprite->set_animation(4);
    }
}
