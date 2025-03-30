#ifndef BMP_TEXTURE_H
#define BMP_TEXTURE_H

#include <SDL3/SDL.h>

#include <string>

#define FUNC_CHEESE   "cheese"

#define RENDER_CHEESE std::string("render/" FUNC_CHEESE "/")

void load_render_functions();
SDL_Texture *load_bmp_texture(const std::string &bmp_path);
void free_textures();

#endif
