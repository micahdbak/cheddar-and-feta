#include "bmp_texture.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "game.h"

#include <iostream>
#include <unordered_map>

static std::unordered_map<std::string, SDL_Texture *(*)(const std::string &)> render_functions;
static std::unordered_map<std::string, SDL_Texture *> bmp_textures;

static SDL_Texture *render_cheese(const std::string &args) {
    int amount = 0;
    sscanf(args.c_str(), "%d", &amount);

    SDL_Texture *cheese_texture = load_bmp_texture("sprites/cheese.bmp");

    int w = 18;
    int h = 12 + ((amount-1) / 4) * 6;

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (texture == nullptr) {
        std::cerr << "render_cheese error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
    SDL_RenderClear(renderer); // make sure texture is cleared

    int x = 1;
    int y = h - 12;
    SDL_FRect src, dst;
    int _amount = amount;

    do {
        int cheese_i = _amount > 3 ? 3 : _amount - 1;
        _amount -= 4;

        src = { float(cheese_i * 16.0f), 0.0f, 16.0f, 12.0f };
        dst = { float(x), float(y), 16.0f, 12.0f };

        SDL_RenderTexture(renderer, cheese_texture, &src, &dst);

        y -= 6;
        x += SDL_rand(2) ? 1 : -1;
        if (x != cnf_clamp(x, 0, 2)) x = 1;
    } while (_amount > 0);
    SDL_SetRenderTarget(renderer, game->screen);

    return texture;
}

void load_render_functions() {
    render_functions[FUNC_CHEESE] = &render_cheese;
}

SDL_Texture *load_bmp_texture(const std::string &bmp_path) {
    if (bmp_textures.find(bmp_path) != bmp_textures.end()) {
        return bmp_textures[bmp_path];
    }

    char buff[1024];
    sscanf(bmp_path.c_str(), "%1023[^/]", buff);

    if (std::string(buff) == "render") {
        size_t offset = strlen(buff) + 1; // + 1 for '/'
        sscanf(bmp_path.c_str() + offset, "%1023[^/]", buff);

        if (render_functions.find(buff) == render_functions.end()) {
            std::cerr << "load_bmp_texture error: render function " << buff << " does not exist." << std::endl;
            std::exit(1);
        }

        char args[1024];
        offset += strlen(buff) + 1;
        sscanf(bmp_path.c_str() + offset, "%1023[^/]", args);

        SDL_Texture *texture = render_functions[buff](args);
        bmp_textures[bmp_path] = texture;
        return texture;
    }

    SDL_Surface *surface = SDL_LoadBMP(bmp_path.c_str());
    if (surface == nullptr) {
        std::cerr << "SDL_LoadBMP error: " << bmp_path << " does not exist." << std::endl;
        char *arr = nullptr;
        int thing = *arr;
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    bmp_textures[bmp_path] = texture;
    return texture;
}

void free_textures() {
    for (auto pair : bmp_textures) {
        SDL_DestroyTexture(pair.second);
    }
}
