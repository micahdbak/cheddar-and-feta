#include "game.h"

#include <algorithm> // std::lower_bound

void Game::set_view(int x, int y) {
    this->corner_x = x - SCREEN_WIDTH/2;
    this->corner_y = y - SCREEN_HEIGHT/2;
}

void Game::push_sprite(SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *dst_rect, int depth_offset) {
    int sprite_y = 0;
    if (dst_rect != nullptr)
        sprite_y = int(dst_rect->y) + depth_offset;

    SpriteRender sprite;
    sprite.texture = texture;
    sprite.src_rect = src_rect;
    sprite.dst_rect = dst_rect;
    sprite.y = sprite_y;

    // std::lower_bound performs a binary search (log n time complexity)
    auto it = std::lower_bound(this->sprites.begin(), this->sprites.end(), sprite);
    this->sprites.insert(it, sprite);
}
