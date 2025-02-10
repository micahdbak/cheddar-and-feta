#include "cheese.h"
#include "game.h"
#include "mouse.h"

#include <iostream>

static SDL_Texture *cheese_texture = nullptr;
static int cheese_texture_refs = 0;

Cheese::Cheese(float x, float y, int amount) {
    this->x = x;
    this->y = y;
    this->amount = amount;
    this->render_cheese();
    cheese_texture_refs++;
}

Cheese::~Cheese() {
    SDL_DestroyTexture(this->texture);

    cheese_texture_refs--;
    if (cheese_texture_refs <= 0 && cheese_texture != nullptr) {
        cheese_texture_refs = 0;
        SDL_DestroyTexture(cheese_texture);
        cheese_texture = nullptr;
    }
}

void Cheese::render_cheese() {
    if (cheese_texture == nullptr) {
        SDL_Surface *cheese_surface = SDL_LoadBMP("sprites/cheese.bmp");
        if (cheese_surface == nullptr) {
            std::cerr << "Cheese::Cheese error: '" << "sprites/cheese.bmp" << "' does not exist." << std::endl;
            exit(1);
        }

        cheese_texture = SDL_CreateTextureFromSurface(renderer, cheese_surface);
        if (cheese_texture == nullptr) {
            std::cerr << "Cheese::Cheese SDL_CreateTextureFromSurface error: " << SDL_GetError() << std::endl;
            exit(1);
        }
        SDL_DestroySurface(cheese_surface);
    }

    int w = 18;
    int h = 12 + ((this->amount-1) / 4) * 6;

    if (this->texture == nullptr) {
        this->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        if (this->texture == nullptr) {
            std::cerr << "Cheese::Cheese SDL_CreateTexture error: " << SDL_GetError() << std::endl;
            exit(1);
        }
        this->depth_offset = h - 2;
    } else {
        h = this->texture->h;
    }
    this->dst_rect.w = float(w);
    this->dst_rect.h = float(h);

    SDL_SetRenderTarget(renderer, this->texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderClear(renderer); // make sure this->texture is cleared

    int _x = 1;
    int _y = h - 12;
    SDL_FRect src, dst;
    int _amount = this->amount;

    do {
        int cheese_i = _amount >= 4 ? 3 : _amount - 1;

        src = { float(cheese_i * 16.0f), 0.0f, 16.0f, 12.0f };
        dst = { float(_x), float(_y), 16.0f, 12.0f };

        SDL_RenderTexture(renderer, cheese_texture, &src, &dst);

        _amount -= 4;
        _y -= 6;
        _x += SDL_rand(2) ? 1 : -1;
        if (_x != cnf_clamp(_x, 0, 2)) _x = 1;
    } while (_amount > 0);
    SDL_SetRenderTarget(renderer, game->screen);
}

void Cheese::step() {
    if (distance_between_points(this->x, this->y, mouse->x, mouse->y) < 8.0f) {
        int available_amount = mouse->max_cheese - mouse->cheese;
        if (available_amount >= this->amount) {
            mouse->cheese += this->amount;
            game->delete_object = true;
            return;
        } else if (available_amount > 0) {
            mouse->cheese += available_amount;
            this->amount -= available_amount;
            this->render_cheese();
        } // else, do nothing
    }

    this->dst_rect.x = float(this->x - game->corner_x - 9);
    this->dst_rect.y = float(this->y - game->corner_y - this->depth_offset);
    game->push_sprite(this->texture, NULL, &this->dst_rect, this->depth_offset);
}
