#include <SDL3/SDL.h>
#include <iostream>
#include <cstdlib>

#include "game.h"
#include "keyboard.h"

Game::Game() {
    this->screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!this->screen) {
        std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_SetTextureScaleMode(this->screen, SDL_SCALEMODE_NEAREST);

    this->mouse = new Sprite("mouse.bmp", 24, 24, 250);
}

Game::~Game() {
    delete this->mouse;
    this->mouse = nullptr;
    SDL_DestroyTexture(this->screen);
    this->screen = nullptr;
}

void Game::step() {
    Uint64 current_ticks = SDL_GetTicks();
    this->delta = float(current_ticks - this->last_ticks) / 1000.0f;

    this->mouse->update_frame();

    if (keyboard.is_down(SDLK_DOWN)) {
        if (keyboard.is_down(SDLK_RIGHT))
            this->mouse->set_animation(1);
        else if (keyboard.is_down(SDLK_LEFT))
            this->mouse->set_animation(7);
        else
            this->mouse->set_animation(0);
    } else if (keyboard.is_down(SDLK_UP)) {
        if (keyboard.is_down(SDLK_RIGHT))
            this->mouse->set_animation(3);
        else if (keyboard.is_down(SDLK_LEFT))
            this->mouse->set_animation(5);
        else
            this->mouse->set_animation(4);
    } else if (keyboard.is_down(SDLK_RIGHT))
        this->mouse->set_animation(2);
    else if (keyboard.is_down(SDLK_LEFT))
        this->mouse->set_animation(6);

    SDL_FRect dst_rect;

    dst_rect.x = 32.0f;
    dst_rect.y = 32.0f;
    dst_rect.w = 48.0f;
    dst_rect.h = 48.0f;

    SDL_SetRenderTarget(renderer, screen);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, this->mouse->texture, &(this->mouse->frame), &dst_rect);
    SDL_SetRenderTarget(renderer, NULL);
}
