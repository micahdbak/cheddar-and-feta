#ifndef BMP_TEXTURE_H
#define BMP_TEXTURE_H

#include <SDL3/SDL.h>

#include <string>

SDL_Texture *load_bmp_texture(const std::string &bmp_path);
void free_textures();

#endif
