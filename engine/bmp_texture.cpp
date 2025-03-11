#include "bmp_texture.h"
#include "game.h"

#include <iostream>
#include <unordered_map>

static std::unordered_map<std::string, SDL_Texture *> bmp_textures;

SDL_Texture *load_bmp_texture(const std::string &bmp_path) {
    if (bmp_textures.contains(bmp_path)) {
        return bmp_textures[bmp_path];
    }

    SDL_Surface *surface = SDL_LoadBMP(bmp_path.c_str());
    if (surface == nullptr) {
        std::cerr << "SDL_LoadBMP error: " << bmp_path << " does not exist." << std::endl;
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    bmp_textures[bmp_path] = texture;
    return texture;
}

void free_textures() {
    for (auto pair : bmp_textures) {
        SDL_DestroyTexture(pair.second);
    }
}
