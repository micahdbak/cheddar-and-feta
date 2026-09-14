#include "icon.h"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>

#include "game.h"
#include "renderer.h"

SDL_Texture* icons = nullptr;

static const unsigned char icons_bmp[] = {
#embed "icons.bmp"
};

void load_icons() {
  thoom::Renderer* renderer = thoom::Renderer::instance;

  SDL_IOStream* io = SDL_IOFromConstMem(icons_bmp, sizeof(icons_bmp));
  SDL_Surface* surface = SDL_LoadBMP_IO(io, true);

  if (surface == nullptr) {
    std::cerr << "load_icons error: " << SDL_GetError() << std::endl;
    exit(1);
  }

  icons = renderer->create_texture_from_surface(surface, SDL_SCALEMODE_NEAREST);
  SDL_DestroySurface(surface);
}

void draw_icon(SDL_FRect icon, SDL_Texture* dst, SDL_FRect* dst_rect) {
  SDL_Renderer* sdl_renderer = thoom::Renderer::instance->sdl_renderer;

  SDL_SetRenderTarget(sdl_renderer, dst);
  SDL_RenderTexture(sdl_renderer, icons, &icon, dst_rect);
  SDL_SetRenderTarget(sdl_renderer, thoom::Renderer::instance->screen);
}

void push_icon(SDL_FRect icon_rect, float x, float y, SDL_FRect* src_rect,
               SDL_FRect* dst_rect) {
  *src_rect = icon_rect;

  dst_rect->x = x - (src_rect->w / 2.0f);
  dst_rect->y = y - (src_rect->h / 2.0f);
  dst_rect->w = src_rect->w;
  dst_rect->h = src_rect->h;

  thoom::game->push_sprite("sprites/icons.bmp", icons, src_rect, dst_rect,
                           THOOM_SCREEN_HEIGHT);
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

  thoom::game->push_sprite("sprites/icons.bmp", icons, src_rect, dst_rect,
                           THOOM_SCREEN_HEIGHT);
}
