#include "render_functions.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "colour.h"
#include "font.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"
#include "thoom/macros.h"
#include "window.h"

static SDL_Texture* render_cheese(const std::string& args) {
  int amount = 0;
  if (1 != sscanf(args.c_str(), "%d", &amount)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  SDL_Texture* cheese_texture =
      thoom::Texture::instance->load("sprites/cheese.bmp");

  int w = 18;
  int h = 12 + ((amount - 1) / 4) * 6;

  SDL_Texture* texture = thoom::Texture::instance->create_texture(
      w, h, SDL_PIXELFORMAT_RGBA8888, SDL_SCALEMODE_PIXELART);

  if (texture == nullptr) {
    std::cerr << "render_cheese error: " << SDL_GetError() << std::endl;
    exit(1);
  }

  thoom::Renderer::instance->clear(texture, thoom::kMask);

  int x = 1;
  int y = h - 12;
  SDL_FRect src, dst;
  int _amount = amount;

  do {
    int cheese_i = _amount > 3 ? 3 : _amount - 1;
    _amount -= 4;

    src = {float(cheese_i * 16.0f), 0.0f, 16.0f, 12.0f};
    dst = {float(x), float(y), 16.0f, 12.0f};

    thoom::Renderer::instance->draw_texture(texture, cheese_texture, &src,
                                            &dst);

    y -= 6;
    x += SDL_rand(2) ? 1 : -1;
    if (x != THOOM_CLAMP(x, 0, 2)) x = 1;
  } while (_amount > 0);

  return texture;
}

static SDL_Texture* render_credits(const std::string& args) {
  int ants = 0, drones = 0, tanks = 0, agents = 0, queen_time = 0;
  if (5 != sscanf(args.c_str(), "%d.%d.%d.%d.%d", &ants, &drones, &tanks,
                  &agents, &queen_time)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  SDL_Texture* texture = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_PIXELART);
  if (texture == nullptr) {
    std::cerr << "render_credits error: " << SDL_GetError() << std::endl;
    exit(1);
  }

  thoom::Renderer::instance->clear(texture, thoom::kMask);

  thoom::Renderer::instance->draw_text(texture, TITLE_FONT, "Cheddar & Feta",
                                       120, 24, 0,
                                       thoom::Colour(24, 24, 24, 255));
  std::string results =
      std::string("}} Results }}\n\n") + "Ant Kills:\t" + std::to_string(ants) +
      "\tDrone Kills:\t" + std::to_string(drones) + "\n" + "Tank Kills:\t" +
      std::to_string(tanks) + "\tAgent Kills:\t" + std::to_string(agents) +
      "\n\n" + "Time to Kill the Queen:\t\t" + std::to_string(queen_time) +
      " (s)\n" + "Total Time:\t\t\t" +
      std::to_string(thoom::Scene::instance->ticks / 1000) + " (s)\n\n" +
      "}} Credits }}\n\n" + "Made with { by Micah Baker\n\n" +
      "Thanks for playing!\n(Press [Enter] to return to Main Menu.)";
  thoom::Renderer::instance->draw_text(texture, DEFAULT_FONT, results, 64, 40,
                                       0, thoom::Colour(24, 24, 24, 255));

  return texture;
}

void register_render_functions() {
  thoom::Texture::instance->register_render_function(FUNC_CHEESE,
                                                     render_cheese);
  thoom::Texture::instance->register_render_function(FUNC_CREDITS,
                                                     render_credits);
}

std::string credits_args(int ants, int drones, int tanks, int agents,
                         int queen_time) {
  char buf[256];
  snprintf(buf, sizeof(buf), "%d.%d.%d.%d.%d", ants, drones, tanks, agents,
           queen_time);
  return std::string(buf);
}
