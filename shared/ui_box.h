#pragma once

#include <SDL3/SDL.h>

extern SDL_Texture* ui_box;

void load_ui_box();
void draw_ui_box(int type, SDL_Texture* dst, SDL_FRect* rect);
