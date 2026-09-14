#pragma once

#include <SDL3/SDL.h>

#define CHEDDAR_ICON SDL_FRect{0.05f, 0.05f, 16.0f, 16.0f}
#define FETA_ICON SDL_FRect{16.05f, 0.05f, 16.0f, 16.0f}
#define NOT_CONNECTED_ICON SDL_FRect{32.05f, 0.05f, 16.0f, 16.0f}
#define WAITING_FOR_PEER_ICON SDL_FRect{48.05f, 0.05f, 16.0f, 16.0f}
#define SKULL_AND_BONES_ICON SDL_FRect{0.05f, 16.05f, 16.0f, 16.0f}
#define EDITOR_OBJECT_ICON SDL_FRect{16.05f, 16.05f, 16.0f, 16.0f}
#define DEBUG_HITBOX_ICON SDL_FRect{32.05f, 16.05f, 16.0f, 16.0f}
#define DEBUG_HURTBOX_ICON SDL_FRect{48.05f, 16.05f, 16.0f, 16.0f}
#define HEALTH_100_ICON SDL_FRect{0.05f, 32.05f, 16.0f, 8.0f}
#define HEALTH_75_ICON SDL_FRect{16.05f, 32.05f, 16.0f, 8.0f}
#define HEALTH_50_ICON SDL_FRect{32.05f, 32.05f, 16.0f, 8.0f}
#define HEALTH_25_ICON SDL_FRect{48.05f, 32.05f, 16.0f, 8.0f}
#define ITEM_COUNT_ICON SDL_FRect{16.05f, 40.05f, 16.0f, 16.0f}
#define ITEM_COUNT_ICON_SHD SDL_FRect{32.05f, 40.05f, 16.0f, 16.0f}
#define ITEM_COOLDOWN_0 SDL_FRect{0.05f, 56.05f, 16.0f, 16.0f}
#define ITEM_COOLDOWN_1 SDL_FRect{16.05f, 56.05f, 16.0f, 16.0f}
#define ITEM_COOLDOWN_2 SDL_FRect{32.05f, 56.05f, 16.0f, 16.0f}
#define ITEM_COOLDOWN_3 SDL_FRect{48.05f, 56.05f, 16.0f, 16.0f}
#define ITEM_COOLDOWN_4 SDL_FRect{64.05f, 56.05f, 16.0f, 16.0f}
#define ITEM_COOLDOWN_5 SDL_FRect{80.05f, 56.05f, 16.0f, 16.0f}
#define ITEM_COOLDOWN_6 SDL_FRect{96.05f, 56.05f, 16.0f, 16.0f}
#define ITEM_COOLDOWN_7 SDL_FRect{112.05f, 56.05f, 16.0f, 16.0f}

extern SDL_Texture* icons;

void load_icons();
void draw_icon(SDL_FRect icon, SDL_Texture* dst, SDL_FRect* dst_rect);
void push_icon(SDL_FRect icon_rect, float x, float y, SDL_FRect* src_rect,
               SDL_FRect* dst_rect);
void push_health_bar(int health, int max_health, float x, float y,
                     SDL_FRect* src_rect, SDL_FRect* dst_rect);
