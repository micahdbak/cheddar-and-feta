#include <SDL3/SDL.h>
#include <iostream>
#include <cstdlib>

#include "game.h"
#include "map.h"

Game::Game() {
    this->screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!this->screen) {
        std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_SetTextureScaleMode(this->screen, SDL_SCALEMODE_NEAREST);

    SDL_Surface *mono_font_surface = SDL_LoadBMP("sprites/mono.bmp");
    this->mono_font = SDL_CreateTextureFromSurface(renderer, mono_font_surface);
    SDL_SetTextureScaleMode(this->mono_font, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(mono_font_surface);
}

Game::~Game() {
    this->unload();
    SDL_DestroyTexture(this->mono_font);
    SDL_DestroyTexture(this->screen);
    this->screen = nullptr;
}

void Game::load_map(const char *map_path) {
    this->unload();

    Map map;
    map.read(map_path);

    for (auto obj : map.objects)
        this->create_object(obj.first, obj.second);

    this->bg = map.bg;
    this->fg = map.fg;

    map.clear();
}

void Game::make_map_rect(int x, int y, int w, int h, SDL_FRect *src_rect, SDL_FRect *dst_rect) {
    int src_x = x - (SCREEN_WIDTH/2);
    int src_y = y - (SCREEN_HEIGHT/2);

    if (src_x < 0) {
        dst_rect->x = float(-1 * src_x);
        dst_rect->w = float(min(SCREEN_WIDTH, w));
        src_rect->x = 0.0f;
    } else {
        dst_rect->x = 0.0f;
        dst_rect->w = float(min(SCREEN_WIDTH, w - src_x));
        src_rect->x = float(src_x);
    }

    if (src_y < 0) {
        dst_rect->y = float(-1 * src_y);
        dst_rect->h = float(min(SCREEN_HEIGHT, h));
        src_rect->y = 0.0f;
    } else {
        dst_rect->y = 0.0f;
        dst_rect->h = float(min(SCREEN_HEIGHT, h - src_y));
        src_rect->y = float(src_y);
    }

    src_rect->w = dst_rect->w;
    src_rect->h = dst_rect->h;
}

void Game::create_object(const std::string &obj_id, const std::string &options) {
    if (!this->factories.contains(obj_id)) {
        std::cerr << "Game::create_object error: '" << obj_id << "' does not exist" << std::endl;
        return;
    }

    Object *obj = this->factories[obj_id]->create(options);
    if (obj != nullptr)
        this->objects.push_back(obj);
}

void Game::draw_text(const std::string &str, int x, int y) {
    Text text;
    text.str = str;
    text.x = x;
    text.y = y;
    texts.push(text);
}

void Game::step() {
    Uint64 current_ticks = SDL_GetTicks();
    this->delta = float(current_ticks - this->last_ticks) / 1000.0f;
    this->last_ticks = current_ticks;

    SDL_SetRenderTarget(renderer, screen);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    // render background
    SDL_FRect map_src, map_dst;
    if (this->bg != nullptr) {
        this->make_map_rect(this->view_x, this->view_y, this->bg->w, this->bg->h, &map_src, &map_dst);
        SDL_RenderTexture(renderer, this->bg, &map_src, &map_dst);
    }

    // render objects to the screen
    for (int i = 0; i < this->objects.size(); i++) {
        Object *obj = this->objects[i];
        obj->step();
        if (obj->texture != nullptr) {
            SDL_RenderTexture(renderer, obj->texture, obj->src_rect, &obj->dst_rect);
        }
    }

    // render foreground
    if (this->fg != nullptr)
        SDL_RenderTexture(renderer, this->fg, &map_src, &map_dst);

    // render all text if there is any
    while (!texts.empty()) {
        Text text = texts.front();
        texts.pop();

        int cols = 16, w = 6, h = 10;

        int longest_line = 0, nlines = 1, cur_line = 0;
        for (auto c : text.str) {
            if (c == '\n') {
                if (cur_line > longest_line) {
                    longest_line = cur_line;
                    cur_line = 0;
                }
                nlines++;
            } else {
                cur_line++;
            }
        }

        if (cur_line > longest_line) {
            longest_line = cur_line;
        }

        SDL_FRect text_area;
        text_area.x = float(text.x);
        text_area.y = float(text.y);
        text_area.w = float(w * longest_line);
        text_area.h = float((h+1) * nlines);
        SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
        SDL_RenderFillRect(renderer, &text_area);

        SDL_FRect src_rect, dst_rect;
        src_rect.w = float(w);
        src_rect.h = float(h);
        dst_rect.y = float(text.y);
        dst_rect.w = float(w);
        dst_rect.h = float(h);
        int x2 = 0, y2 = 0;
        for (int i = 0; i < text.str.size(); i++) {
            if (text.str[i] == '\n') {
                y2++, x2 = 0;
                continue;
            }

            // charsheets start at ' ', orderred same as ASCII
            char c_index = text.str[i] - ' ';
            src_rect.x = float((c_index % cols) * w);
            src_rect.y = float((c_index / cols) * h);
            dst_rect.x = float(text.x + (x2 * w));
            dst_rect.y = float(text.y + (y2 * (h + 1)));
            SDL_RenderTexture(renderer, this->mono_font, &src_rect, &dst_rect);
            x2++;
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
}

void Game::unload() {
    if (!this->objects.empty()) {
        for (int i = 0; i < this->objects.size(); i++) {
            delete this->objects[i];
            this->objects[i] = nullptr;
        }

        this->objects.clear();
    }

    if (this->bg != nullptr) {
        SDL_DestroyTexture(this->bg);
        this->bg = nullptr;
    }

    if (this->fg != nullptr) {
        SDL_DestroyTexture(this->fg);
        this->fg = nullptr;
    }
}
