#ifndef BMP_TEXTURE_H
#define BMP_TEXTURE_H

#include <SDL3/SDL.h>

#include <string>

#define FUNC_CHEESE "cheese"
#define FUNC_CREDITS "credits"

#define RENDER_CHEESE std::string("render/" FUNC_CHEESE "/")
#define RENDER_CREDITS std::string("render/" FUNC_CREDITS "/")

void load_render_functions();
SDL_Texture* load_bmp_texture(const std::string& bmp_path);
void free_textures();

std::string credits_args(int ants, int drones, int tanks, int agents,
                         int queen_time);

#endif
