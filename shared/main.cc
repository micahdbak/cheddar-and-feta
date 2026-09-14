#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "controller.h"
#include "game.h"
#include "hud.h"
#include "icon.h"
#include "renderer.h"
#include "save_data.h"
#include "ui_box.h"

bool thoom::_running, thoom::capture_controls = false;
thoom::Controller thoom::local_controller, thoom::remote_controller,
    thoom::captured_controller;
thoom::Game* thoom::game;
int thoom::_object_id_counter = 0;

int main(int argc, char** argv) {
  thoom::Renderer::init("Thoom Engine", THOOM_SCREEN_WIDTH,
                        THOOM_SCREEN_HEIGHT);
  thoom::Renderer* renderer = thoom::Renderer::instance;

  thoom::_running = true;
  thoom::game = new thoom::Game();
  thoom::game->argc = argc;
  thoom::game->argv = argv;
  thoom::game->init();
  renderer->set_window_title(thoom::game->title.c_str());

  load_ui_box();
  load_icons();
  Hud::init();
  Hud* hud = Hud::instance;

  while (thoom::_running) {
    SDL_Event event;

    thoom::Controller* controller = thoom::capture_controls
                                        ? &thoom::captured_controller
                                        : &thoom::local_controller;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED:
          renderer->update_screen_scale();
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
          thoom::_running = false;
          break;

        default:
          // pass
          break;
      }
    }

    thoom::game->step();

    hud->render();

    renderer->draw_texture(renderer->screen, hud->ui, NULL, NULL);
    renderer->draw_texture(renderer->screen, hud->overlay, NULL, NULL);

    controller->clear_hits();

    renderer->present(thoom::Colour{thoom::game->bg_r, thoom::game->bg_g,
                                    thoom::game->bg_b, 255});
  }

  Hud::shutdown();

  delete thoom::game;
  thoom::game = nullptr;

  SDL_DestroyTexture(ui_box);
  ui_box = nullptr;

  SDL_DestroyTexture(icons);
  icons = nullptr;

  thoom::Renderer::shutdown();

  exit(0);
}
