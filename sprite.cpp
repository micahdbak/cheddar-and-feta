#include <iostream>

#include "game.h"
#include "sprite.h"

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

    this->frame_last_set = SDL_GetTicks();
}

Sprite::~Sprite() {
    SDL_DestroyTexture(this->texture);
}

void Sprite::set_animation(int animation) {
    this->frame.y = float((this->frame_h * animation) % this->sheet_h);
}

void Sprite::update_frame() {
    Uint64 now_ticks = SDL_GetTicks();

    if (now_ticks - this->frame_last_set > this->interval_ms) {
        this->frame.x += float(this->frame_w);
        if (int(this->frame.x) >= this->sheet_w - 1) {
            this->frame.x = 0.0f;
        }
        this->frame_last_set = now_ticks;
    }
}
