#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iostream>
#include <string>

#include "audio/audio.h"
#include "colour.h"
#include "editor.h"
#include "editor_ui.h"
#include "hud/hud.h"
#include "input/input.h"
#include "map/map.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "thoom/thoom.h"
#include "window/window.h"

bool init(int argc, char** argv);

int main(int argc, char** argv) {
  struct hud_def hud = {};

  if (!thoom_init("Thoom Engine", THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT,
                  false, kBackground.val, &hud)) {
    return 1;
  }

  if (!thoom::Scene::init()) {
    thoom_quit();
    return 1;
  }

  if (!init(argc, argv)) {
    thoom::Scene::quit();
    thoom_quit();
    return 1;
  }

  bool running = true;

  while (running) {
    bool quit = false;
    thoom_poll_events(&quit);

    if (quit) {
      running = false;
      break;
    }

    thoom::Audio::instance->step();
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

    thoom::Input::instance->clear_hit_input();
  }

  thoom::Scene::quit();

  thoom_quit();

  return 0;
}

bool init(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Please provide a map to edit via the command line."
              << std::endl;
    return false;
  }

  std::string options;
  for (int i = 1; i < argc; i++) {
    if (i > 1) options += '\n';
    options += argv[i];
  }

  thoom::Scene::ObjectDef editor;
  editor.id = "editor";
  editor.create_object = Editor::create_object;
  thoom::Scene::instance->register_object(editor);
  thoom::Scene::instance->create_object("editor", options);

  std::string title = "Map Editor - " + std::string(argv[1]);
  thoom::Window::instance->set_title(title.c_str());

  return true;
}
