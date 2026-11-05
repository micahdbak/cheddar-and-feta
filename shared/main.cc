#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstddef>
#include <cstdlib>

#include "audio.h"
#include "colour.h"
#include "constants.h"
#include "hud.h"
#include "icon.h"
#include "input.h"
#include "map.h"
#include "peer.h"
#include "render_functions.h"
#include "renderer.h"
#include "scene.h"
#include "sounds.h"
#include "texture.h"
#include "ui_box.h"
#include "window.h"

bool game_is_cheddar = false;

void game_init(int argc, char** argv);

int main(int argc, char** argv) {
  thoom::Window::init("Thoom Engine", THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT);
  thoom::Renderer::init();
  thoom::Audio::init();
  thoom::Texture::init();
  thoom::Input::init();
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

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED:
          thoom::Window::instance->update_screen_scale();
          break;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
          thoom::Input::instance->handle_key_event(event);
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

    if (thoom::Input::instance->world.character == 'p') {
      thoom::Window::instance->save_screenshot("screenshot.bmp");
    }

    Hud::instance->render();

    thoom::Renderer::instance->draw_texture(thoom::Window::instance->screen,
                                            Hud::instance->ui, NULL, NULL);
    thoom::Renderer::instance->draw_texture(thoom::Window::instance->screen,
                                            Hud::instance->overlay, NULL, NULL);

    thoom::Input::instance->clear_hit_input();

    thoom::Window::instance->present(bg);
  }

  Hud::shutdown();

  thoom::Scene::shutdown();

  thoom::Peer::shutdown();
  thoom::Input::shutdown();
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
