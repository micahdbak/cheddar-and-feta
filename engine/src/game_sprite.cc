#include <algorithm>  // std::lower_bound

#include "game.h"

void Game::set_view(int x, int y) {
  this->corner_x = x - SCREEN_WIDTH / 2;
  this->corner_y = y - SCREEN_HEIGHT / 2;
}

void Game::push_sprite(const std::string& tex_id, SDL_Texture* texture,
                       SDL_FRect* src_rect, SDL_FRect* dst_rect,
                       int depth_offset) {
  int sprite_y = 0;

  if (dst_rect != nullptr) sprite_y = (int)dst_rect->y + depth_offset;

  SpriteRender sprite;
  sprite.tex_id = tex_id;
  sprite.texture = texture;
  sprite.src_rect = src_rect;
  sprite.dst_rect = dst_rect;
  sprite.y = sprite_y;

  // std::lower_bound performs a binary search (log n time complexity)
  auto it =
      std::lower_bound(this->sprites.begin(), this->sprites.end(), sprite);
  this->sprites.insert(it, sprite);
}

void Game::push_icon(SDL_FRect icon_rect, float x, float y, SDL_FRect* src_rect,
                     SDL_FRect* dst_rect) {
  *src_rect = icon_rect;

  dst_rect->x = x - (src_rect->w / 2.0f);
  dst_rect->y = y - (src_rect->h / 2.0f);
  dst_rect->w = src_rect->w;
  dst_rect->h = src_rect->h;

  this->push_sprite("sprites/icons.bmp", this->icons, src_rect, dst_rect,
                    SCREEN_HEIGHT);
}

void Game::push_health_bar(int health, int max_health, float x, float y,
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

  game->push_sprite("sprites/icons.bmp", this->icons, src_rect, dst_rect,
                    SCREEN_HEIGHT);
}
