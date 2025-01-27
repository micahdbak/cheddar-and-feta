#include "font.h"
#include "game.h"

#include <iostream>

Font::Font(const char *font_path, int w, int h, int default_w, const std::unordered_map<char, int> &special_w) {
    SDL_Surface *font_surface = SDL_LoadBMP(font_path);
    if (font_surface == nullptr) {
        std::cerr << "Font::Font error: '" << font_path << "' does not exist." << std::endl;
        exit(1);
    }
    this->texture = SDL_CreateTextureFromSurface(renderer, font_surface);
    SDL_SetTextureScaleMode(this->texture, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(font_surface);

    // create all src_rect's
    for (int i = 0; i < NUM_DISPLAYABLE_CHARS; i++) {
        this->src_rect[i].x = float((i % FONT_SHEET_COLS) * w);
        this->src_rect[i].y = float((i / FONT_SHEET_COLS) * h);
        this->src_rect[i].w = float(default_w);
        this->src_rect[i].h = float(h);

        // see if this character has a special width
        char c = char(i) + ' ';
        if (special_w.contains(c)) {
            this->src_rect[i].w = float(special_w.at(c));
        }
    }
}


Font::~Font() {
    if (this->texture != nullptr) {
        SDL_DestroyTexture(this->texture);
        this->texture = nullptr;
    }
}
