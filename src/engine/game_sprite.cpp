#include "game.h"

#include <algorithm> // std::lower_bound

void Game::set_view(int x, int y) {
    this->corner_x = x - SCREEN_WIDTH/2;
    this->corner_y = y - SCREEN_HEIGHT/2;
}

void Game::push_sprite(const std::string &tex_id, SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *dst_rect, int depth_offset) {
    int sprite_y = 0;
    if (dst_rect != nullptr)
        sprite_y = int(dst_rect->y) + depth_offset;

    SpriteRender sprite;
    sprite.tex_id = tex_id;
    sprite.texture = texture;
    sprite.src_rect = src_rect;
    sprite.dst_rect = dst_rect;
    sprite.y = sprite_y;

    // std::lower_bound performs a binary search (log n time complexity)
    auto it = std::lower_bound(this->sprites.begin(), this->sprites.end(), sprite);
    this->sprites.insert(it, sprite);
}

static SDL_FRect _draw_icon_rects[100];
static int _draw_icon_rects_i = 0;

void Game::push_icon(SDL_FRect icon, SDL_FRect *dst_rect) {
    SDL_FRect *src_rect = _draw_icon_rects + _draw_icon_rects_i;
    _draw_icon_rects_i++;
    if (_draw_icon_rects_i >= sizeof(_draw_icon_rects))
        _draw_icon_rects_i = 0;

    *src_rect = icon;
    this->push_sprite("sprites/icons.bmp", this->icons, src_rect, dst_rect, 1000);
}
