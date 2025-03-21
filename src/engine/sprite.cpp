#include "game.h"
#include "sprite.h"
#include "bmp_texture.h"

#include <iostream>

Sprite::Sprite(const char *bmp_path, int frame_w, int frame_h, int interval_ms):
    frame_w(frame_w), frame_h(frame_h), interval_ms(interval_ms) {
    this->texture = load_bmp_texture(bmp_path);
    this->sheet_w = this->texture->w;
    this->sheet_h = this->texture->h;
    SDL_SetTextureScaleMode(this->texture, SDL_SCALEMODE_NEAREST);

    this->frame.x = 0.0f;
    this->frame.y = 0.0f;
    this->frame.w = float(frame_w);
    this->frame.h = float(frame_h);

    this->frame_i = 0;
    this->frame_last_set = game->ticks;
}

Sprite::~Sprite() {
    this->texture = nullptr;
}

void Sprite::set_animation(int animation) {
    if (this->animation == animation) return;

    this->animation = animation;
    this->frame.y = float(this->frame_h * animation);
}

void Sprite::update_frame() {
    if (game->ticks - this->frame_last_set > this->interval_ms) {
        this->frame_i++;
        if (this->frame_i * this->frame_w >= this->sheet_w) {
            this->frame_i = 0;
        }
        this->frame.x = float(this->frame_i * this->frame_w);
        this->frame_last_set = game->ticks;
    }
}

void Sprite::set_frame(int frame_i) {
    this->frame_i = frame_i;
    this->frame.x = float(this->frame_i * this->frame_w);
    this->frame_last_set = game->ticks;
}
