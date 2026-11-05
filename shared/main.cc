#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "audio.h"
#include "constants.h"
#include "controller.h"
#include "hud.h"
#include "icon.h"
#include "peer.h"
#include "render_functions.h"
#include "renderer.h"
#include "save_data.h"
#include "scene.h"
#include "sounds.h"
#include "texture.h"
#include "ui_box.h"
#include "window.h"

bool thoom::capture_controls = false;
thoom::Controller thoom::local_controller, thoom::remote_controller,
    thoom::captured_controller;
bool game_is_cheddar = false;

void game_init(int argc, char** argv);

int main(int argc, char** argv) {
  thoom::Window::init("Thoom Engine", THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT);
  thoom::Renderer::init();
  thoom::Audio::init();
  thoom::Texture::init();
  register_render_functions();

  thoom::Scene::init();
  game_init(argc, argv);

  load_ui_box();
  load_icons();
  Hud::init();

  load_sounds();

  bool running = true;

  while (running) {
    SDL_Event event;

    thoom::Controller* controller = thoom::capture_controls
                                        ? &thoom::captured_controller
                                        : &thoom::local_controller;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED:
          thoom::Window::instance->update_screen_scale();
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
          running = false;
          break;

        default:
          // pass
          break;
      }
    }

    thoom::Audio::instance->step();
    thoom::Scene::instance->step();

    thoom::Map* map = thoom::Scene::instance->map;
    thoom::Colour bg = map != nullptr ? map->background : thoom::kBlack;
    thoom::Window::instance->clear_screen(bg);
    thoom::Renderer::instance->draw_map_background(map);
    thoom::Renderer::instance->draw_sprites();
    thoom::Renderer::instance->draw_map_foreground(map);

    if (thoom::local_controller.c == 'p' ||
        thoom::captured_controller.c == 'p') {
      thoom::local_controller.c = NO_CHAR;
      thoom::Window::instance->save_screenshot("screenshot.bmp");
    }

    Hud::instance->render();

    thoom::Renderer::instance->draw_texture(thoom::Window::instance->screen,
                                            Hud::instance->ui, NULL, NULL);
    thoom::Renderer::instance->draw_texture(thoom::Window::instance->screen,
                                            Hud::instance->overlay, NULL, NULL);

    controller->clear_hits();

    thoom::Window::instance->present(bg);
  }

  Hud::shutdown();

  thoom::Scene::shutdown();

  thoom::Peer::shutdown();
  thoom::Texture::shutdown();
  thoom::Audio::shutdown();

  SDL_DestroyTexture(ui_box);
  ui_box = nullptr;

  SDL_DestroyTexture(icons);
  icons = nullptr;

  thoom::Renderer::shutdown();
  thoom::Window::shutdown();

  exit(0);
}
