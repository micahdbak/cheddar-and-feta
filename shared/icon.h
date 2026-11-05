#pragma once

#include <SDL3/SDL.h>

constexpr SDL_FRect CHEDDAR_ICON{0.05f, 0.05f, 16.0f, 16.0f};
constexpr SDL_FRect FETA_ICON{16.05f, 0.05f, 16.0f, 16.0f};
constexpr SDL_FRect NOT_CONNECTED_ICON{32.05f, 0.05f, 16.0f, 16.0f};
constexpr SDL_FRect WAITING_FOR_PEER_ICON{48.05f, 0.05f, 16.0f, 16.0f};
constexpr SDL_FRect SKULL_AND_BONES_ICON{0.05f, 16.05f, 16.0f, 16.0f};
constexpr SDL_FRect EDITOR_OBJECT_ICON{16.05f, 16.05f, 16.0f, 16.0f};
constexpr SDL_FRect DEBUG_HITBOX_ICON{32.05f, 16.05f, 16.0f, 16.0f};
constexpr SDL_FRect DEBUG_HURTBOX_ICON{48.05f, 16.05f, 16.0f, 16.0f};
constexpr SDL_FRect HEALTH_100_ICON{0.05f, 32.05f, 16.0f, 8.0f};
constexpr SDL_FRect HEALTH_75_ICON{16.05f, 32.05f, 16.0f, 8.0f};
constexpr SDL_FRect HEALTH_50_ICON{32.05f, 32.05f, 16.0f, 8.0f};
constexpr SDL_FRect HEALTH_25_ICON{48.05f, 32.05f, 16.0f, 8.0f};
constexpr SDL_FRect ITEM_COUNT_ICON{16.05f, 40.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COUNT_ICON_SHD{32.05f, 40.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COOLDOWN_0{0.05f, 56.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COOLDOWN_1{16.05f, 56.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COOLDOWN_2{32.05f, 56.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COOLDOWN_3{48.05f, 56.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COOLDOWN_4{64.05f, 56.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COOLDOWN_5{80.05f, 56.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COOLDOWN_6{96.05f, 56.05f, 16.0f, 16.0f};
constexpr SDL_FRect ITEM_COOLDOWN_7{112.05f, 56.05f, 16.0f, 16.0f};

extern SDL_Texture* icons;

void load_icons();
void draw_icon(SDL_FRect icon, SDL_Texture* dst, SDL_FRect* dst_rect);
void push_icon(SDL_FRect icon_rect, float x, float y, SDL_FRect* src_rect,
               SDL_FRect* dst_rect);
void push_health_bar(int health, int max_health, float x, float y,
                     SDL_FRect* src_rect, SDL_FRect* dst_rect);
