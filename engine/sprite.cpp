#include "game.h"
#include "sprite.h"

#include <iostream>

Sprite::Sprite(const char *bmp_path, int frame_w, int frame_h, int interval_ms):
    frame_w(frame_w), frame_h(frame_h), interval_ms(interval_ms) {
    SDL_Surface *sprite_surface = SDL_LoadBMP(bmp_path);
    if (!sprite_surface) {
        std::cerr << "SDL_LoadBMP error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    this->sheet_w = sprite_surface->w;
    this->sheet_h = sprite_surface->h;

    this->texture = SDL_CreateTextureFromSurface(renderer, sprite_surface);
    SDL_DestroySurface(sprite_surface);
    if (!this->texture) {
        std::cerr << "SDL_CreateTextureFromSurface error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_SetTextureScaleMode(this->texture, SDL_SCALEMODE_NEAREST);

    this->frame.x = 0.0f;
    this->frame.y = 0.0f;
    this->frame.w = float(frame_w);
    this->frame.h = float(frame_h);

    this->frame_i = 0;
    this->frame_last_set = game->ticks;
}

Sprite::~Sprite() {
    SDL_DestroyTexture(this->texture);
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
}

void Sprite::set_interval_ms(int interval_ms) {
    this->interval_ms = interval_ms;
}
