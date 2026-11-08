#include "game.h"

#include <SDL3/SDL.h>

#include <cstdint>

#include "audio/audio.h"
#include "colour.h"
#include "hud/hud.h"
#include "icon.h"
#include "input/input.h"
#include "map/map.h"
#include "peer/peer.h"
#include "render_functions.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "sounds.h"
#include "thoom/thoom.h"
#include "window/window.h"

bool game_run(const GameDef& def, int argc, char** argv) {
  if (!thoom_init(def.title, THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, def.peer,
                  def.offerer, def.background, def.hud)) {
    return false;
  }

  register_render_functions();

  if (!thoom::Scene::init()) {
    thoom_quit();
    return false;
  }

  if (thoom::Peer::instance != nullptr) {
    thoom::Scene::instance->local_pid = thoom::Peer::instance->get_pid();
  }

  if (!def.init(argc, argv)) {
    thoom::Scene::quit();
    thoom_quit();
    return false;
  }

  load_icons();
  load_sounds();

  for (;;) {
    bool quit = false;
    thoom_poll_events(&quit);

    if (quit) {
      break;
    }

    thoom::Audio::instance->step();

    uint64_t ticks = thoom::Scene::instance->ticks;
    thoom::Scene::instance->step();

    thoom::Map* map = thoom::Scene::instance->map;
    thoom::Renderer::instance->draw_world(map);
    thoom::Hud::instance->render();
    thoom::Hud::instance->draw();

    if (thoom::Input::instance->world.character == 'p') {
      thoom::Window::instance->save_screenshot("screenshot.bmp");
    }

    thoom::Colour bg = map != nullptr ? map->background : thoom::kBlack;
    thoom::Window::instance->present(bg);

    if (thoom::Scene::instance->ticks != ticks) {
      thoom::Input::instance->clear_hit_input();
    }
  }

  thoom::Scene::quit();

  SDL_DestroyTexture(icons);
  icons = nullptr;

  thoom_quit();

  return true;
}
