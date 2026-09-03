#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdlib>
#include <iostream>

#include "controller.h"
#include "game.h"
#include "save_data.h"

#if defined(__SANITIZE_ADDRESS__) || \
    (defined(__has_feature) && __has_feature(address_sanitizer))
#include <sanitizer/lsan_interface.h>
#define LSAN_DISABLE() __lsan_disable()
#define LSAN_ENABLE() __lsan_enable()
#else
#define LSAN_DISABLE()
#define LSAN_ENABLE()
#endif

SDL_Window* window;
SDL_Renderer* renderer;

bool _running, capture_controls = false;
Controller local_controller, remote_controller, captured_controller;
Game* game;

int _object_id_counter = 0;

void cleanup();
void scale_screen_rect(SDL_FRect* screen_rect, const int window_width,
                       const int window_height);

int main(int argc, char** argv) {
  bool init_ok;

  LSAN_DISABLE();
  init_ok = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
  LSAN_ENABLE();

  if (!init_ok) {
    std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
    return 1;
  }

  int window_width = SCREEN_WIDTH * 2, window_height = SCREEN_HEIGHT * 2;
  if (!SDL_CreateWindowAndRenderer(argv[0], window_width, window_height,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    std::cerr << "SDL_CreateWindowAndRenderer error: " << SDL_GetError()
              << std::endl;
    return 1;
  }

  // std::cout << "Renderer: " << SDL_GetRendererName(renderer) << std::endl;

  SDL_SetRenderVSync(renderer, 1);

  if (std::atexit(cleanup) != 0) {
    std::cerr << "main error: couldn't register exit function" << std::endl;
    cleanup();
    return 1;
  }

  SDL_FRect screen_rect;
  scale_screen_rect(&screen_rect, window_width, window_height);

  _running = true;
  game = new Game();
  game->argc = argc;
  game->argv = argv;
  game->init();
  SDL_SetWindowTitle(window, game->title.c_str());

  while (_running) {
    SDL_Event event;

    Controller* controller =
        capture_controls ? &captured_controller : &local_controller;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED:
          SDL_GetWindowSizeInPixels(window, &window_width, &window_height);
          scale_screen_rect(&screen_rect, window_width, window_height);
          break;

        case SDL_EVENT_KEY_DOWN:
          if (!event.key.repeat) {
            controller->handle_key_down(event.key.key);
            controller->last_input = event.key.key;
          }
          break;

        case SDL_EVENT_KEY_UP:
          controller->handle_key_up(event.key.key);
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

    controller->clear_hits();

    SDL_SetRenderDrawColor(renderer, game->bg_r, game->bg_g, game->bg_b, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, game->screen, NULL, &screen_rect);
    SDL_RenderPresent(renderer);
  }

  exit(0);
}

void cleanup() {
  delete game;
  game = nullptr;
  SDL_SetRenderTarget(renderer, NULL);
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
}

void scale_screen_rect(SDL_FRect* screen_rect, const int window_width,
                       const int window_height) {
  static const float screen_aspect_ratio_wtoh =
      float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
  static const float screen_aspect_ratio_htow =
      float(SCREEN_HEIGHT) / float(SCREEN_WIDTH);

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
