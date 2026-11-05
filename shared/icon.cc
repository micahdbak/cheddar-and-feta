#include "icon.h"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>

#include "renderer.h"
#include "texture.h"
#include "window.h"

SDL_Texture* icons = nullptr;

static const unsigned char icons_bmp[] = {
#embed "icons.bmp"
};

void load_icons() {
  icons = thoom::Texture::instance->create_texture_from_bmp_memory(
      icons_bmp, sizeof(icons_bmp), SDL_SCALEMODE_NEAREST);
  if (icons == nullptr) std::exit(1);
}

void draw_icon(SDL_FRect icon, SDL_Texture* dst, SDL_FRect* dst_rect) {
  thoom::Renderer::instance->draw_texture(dst, icons, &icon, dst_rect);
}

void push_icon(SDL_FRect icon_rect, float x, float y, SDL_FRect* src_rect,
               SDL_FRect* dst_rect) {
  *src_rect = icon_rect;

  dst_rect->x = x - (src_rect->w / 2.0f);
  dst_rect->y = y - (src_rect->h / 2.0f);
  dst_rect->w = src_rect->w;
  dst_rect->h = src_rect->h;

  thoom::Renderer::instance->push_sprite("sprites/icons.bmp", icons, src_rect,
                                         dst_rect, THOOM_SCREEN_HEIGHT);
}

void push_health_bar(int health, int max_health, float x, float y,
                     SDL_FRect* src_rect, SDL_FRect* dst_rect) {
  float perc = (float)health / (float)max_health;

  if (perc < 0.26f) {
    *src_rect = HEALTH_25_ICON;
  } else if (perc < 0.51f) {
    *src_rect = HEALTH_50_ICON;
  } else if (perc < 0.76f) {
    *src_rect = HEALTH_75_ICON;
  } else {
    *src_rect = HEALTH_100_ICON;
  }

  dst_rect->x = x - (src_rect->w / 2.0f);
  dst_rect->y = y - (src_rect->h / 2.0f);
  dst_rect->w = src_rect->w;
  dst_rect->h = src_rect->h;

  thoom::Renderer::instance->push_sprite("sprites/icons.bmp", icons, src_rect,
                                         dst_rect, THOOM_SCREEN_HEIGHT);
}
