#include "keyboard.h"
#include "game.h"
#include "save_data.h"

#include <SDL3/SDL.h>

#include <iostream>
#include <cstdlib>

// externs in game.h
SDL_Renderer *renderer;
Game *game;
bool _running;

SDL_Window *window;

Keyboard keyboard;

void cleanup();
void scale_screen_rect(SDL_FRect *screen_rect, const int window_width, const int window_height);

int main(int argc, const char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int window_width = SCREEN_WIDTH*2, window_height = SCREEN_HEIGHT*2;
    if (!SDL_CreateWindowAndRenderer(argv[0], window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        std::cerr << "SDL_CreateWindowAndRenderer error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "Renderer: " << SDL_GetRendererName(renderer) << std::endl;

    SDL_SetRenderVSync(renderer, 1);

    if (std::atexit(cleanup) != 0) {
        std::cerr << "main error: couldn't register exit function" << std::endl;
        cleanup();
        return 1;
    }

    SDL_FRect screen_rect;
    scale_screen_rect(&screen_rect, window_width, window_height);

    int ret = save.load_file(0);
    if (ret == LOAD_TAMPER) {
        save.data["tampered"] = "true";
        std::cout << "tampered - but ignoring" << std::endl;
    }

    _running = true;
    game = new Game();
    game->argc = argc;
    game->argv = argv;
    game->init();
    SDL_SetWindowTitle(window, game->title.c_str());

    while (_running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_WINDOW_RESIZED:
                SDL_GetWindowSizeInPixels(window, &window_width, &window_height);
                scale_screen_rect(&screen_rect, window_width, window_height);
                break;

            case SDL_EVENT_KEY_DOWN:
                if (!event.key.repeat) {
                    keyboard.handle_down(event.key.key);
                }
                break;

            case SDL_EVENT_KEY_UP:
                keyboard.handle_up(event.key.key);
                break;

            case SDL_EVENT_QUIT:
                _running = false;
                break;

            default:
                // pass
                break;
            }
        }

        game->step();
        keyboard.clear_hits();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, game->screen, NULL, &screen_rect);
        SDL_RenderPresent(renderer);
    }

    exit(0);
}

void cleanup() {
    delete game;
    game = nullptr;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void scale_screen_rect(SDL_FRect *screen_rect, const int window_width, const int window_height) {
    static const float screen_aspect_ratio_wtoh = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
    static const float screen_aspect_ratio_htow = float(SCREEN_HEIGHT) / float(SCREEN_WIDTH);

    float window_aspect_ratio = float(window_width) / float(window_height);

    if (window_aspect_ratio > screen_aspect_ratio_wtoh) {
        screen_rect->w = screen_aspect_ratio_wtoh * float(window_height);
        screen_rect->h = float(window_height);
        screen_rect->x = 0.5f * (float(window_width) - screen_rect->w);
        screen_rect->y = 0.0f;
    } else {
        screen_rect->w = float(window_width);
        screen_rect->h = screen_aspect_ratio_htow * float(window_width);
        screen_rect->x = 0.0f;
        screen_rect->y = 0.5f * (float(window_height) - screen_rect->h);
    }
}
