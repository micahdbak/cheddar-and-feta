#include "game.h"
#include "keyboard.h"
#include "map.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>

Game::Game() {
    // create screen texture
    this->screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!this->screen) {
        std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_SetTextureScaleMode(this->screen, SDL_SCALEMODE_NEAREST);

    // create ui texture
    this->ui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!this->ui) {
        std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    this->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

    // load the ui box texture
    SDL_Surface *ui_box_surface = SDL_LoadBMP("sprites/ui_box.bmp");
    if (ui_box_surface == nullptr) {
        std::cerr << "SDL_LoadBMP error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    this->ui_box = SDL_CreateTextureFromSurface(renderer, ui_box_surface);
    if (!this->ui_box) {
        std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_SetTextureScaleMode(this->ui_box, SDL_SCALEMODE_NEAREST);

    // MONO_FONT
    this->fonts.push_back(new Font("fonts/mono.bmp", 6, 10, 6, {}));

    // SMALL_FONT
    this->fonts.push_back(new Font("fonts/small.bmp", 6, 7, 4, {
        { ' ', 3 }, { '!', 2 }, { '#', 6 }, { '%', 5 },
        { '&', 5 }, { '\'', 3 }, { ',', 3 }, { '.', 2 },
        { ':', 2 }, { ';', 3 }, { '<', 6 }, { '>', 6 },
        { '@', 5 }, { 'M', 6 }, { 'N', 5 }, { 'W', 6 },
        { '^', 6 }, { '`', 3 }, { 'i', 2 }, { 'j', 3 },
        { 'l', 2 }, { 'm', 6 }, { 'w', 6 }, { '{', 6 },
        { '|', 6 }, { '}', 6 }, { '~', 6 }
    }));

    // DEFAULT_FONT
    this->fonts.push_back(new Font("fonts/default.bmp", 6, 10, 5, {
        { ' ', 3 }, { '!', 2 }, { '"', 4 }, { '#', 6 },
        { '\'', 3 }, { '(', 4 }, { ')', 4 }, { '*', 4 },
        { '+', 6 }, { ',', 3 }, { '.', 2 }, { '1', 4 },
        { ':', 2 }, { ';', 3 }, { 'I', 4 }, { 'J', 4 },
        { 'M', 6 }, { 'V', 6 }, { 'W', 6 }, { 'X', 6 },
        { '^', 4 }, { '`', 3 }, { 'i', 2 }, { 'j', 3 },
        { 'l', 2 }, { 'm', 6 }, { 'v', 6 }, { 'w', 6 },
        { 'x', 6 }, { '|', 6 }
    }));

    this->ticks = SDL_GetTicks();
    this->last_ticks = this->ticks;
    this->frame_ticks = this->ticks;
}

Game::~Game() {
    this->unload();

    // free fonts
    for (auto font : this->fonts)
        delete font;
    this->fonts.clear();

    // destroy the screen
    SDL_DestroyTexture(this->screen);
    this->screen = nullptr;

    // destroy the ui texture
    SDL_DestroyTexture(this->ui);
    this->ui = nullptr;

    // destroy the ui box texture
    SDL_DestroyTexture(this->ui_box);
    this->ui_box = nullptr;
}

void Game::make_map_rect(int x, int y, int w, int h, SDL_FRect *src_rect, SDL_FRect *dst_rect) const {
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

// below `_sign` and `_point_in_triangle` functions from:
// https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle

static float _sign(SDL_FPoint p1, SDL_FPoint p2, SDL_FPoint p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

static bool _point_in_triangle(SDL_FPoint pt, SDL_FPoint v1, SDL_FPoint v2, SDL_FPoint v3) {
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = _sign(pt, v1, v2);
    d2 = _sign(pt, v2, v3);
    d3 = _sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

bool Game::point_in_collider(float x, float y) const {
    // no collision
    if (this->collision == nullptr)
        return false;

    // flooring is intentional
    int _x = int(x) / this->tile_width;
    int _y = int(y) / this->tile_height;

    // out of map is automatic collision
    if (_x < 0 || _y < 0 || _x >= this->cols || _y >= this->rows)
        return true;

    int coord = (_y * this->cols) + _x;

    // no collider at point or invalid collider
    if (this->collision[coord] < 0 || this->collision[coord] >= n_MapColliders)
        return false;

    Quad quad = this->colliders[this->collision[coord]];
    SDL_FPoint point;
    point.x = x - float(_x * this->tile_width);
    point.y = y - float(_y * this->tile_width);

    // return true if the point lies in either triangles making up the collider's quad
    return _point_in_triangle(point, quad.vertex[0], quad.vertex[1], quad.vertex[2]) ||
           _point_in_triangle(point, quad.vertex[2], quad.vertex[3], quad.vertex[0]);
}

bool Game::in_sight(int x0, int y0, int x1, int y1, int *next_x, int *next_y) const {
    x0 = x0 / this->tile_width;
    y0 = y0 / this->tile_width;
    x1 = x1 / this->tile_width;
    y1 = y1 / this->tile_height;

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    bool next_set = false;

    while (true) {
        // return false if there is a collider in the way
        if (this->collision[(y0 * this->cols) + x0] >= 0)
            return false;

        int e2 = 2 * error;
        if (e2 >= dy) {
            if (x0 == x1) break;
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx) {
            if (y0 == y1) break;
            error = error + dx;
            y0 = y0 + sy;
        }

        if (!next_set && next_x != NULL && next_y != NULL) {
            *next_x = x0 * this->tile_width;
            *next_y = y0 * this->tile_height;
            next_set = true;
        }
    }

    if (!next_set && next_x != NULL && next_y != NULL) {
        *next_x = x0 * this->tile_width;
        *next_y = y0 * this->tile_height;
        next_set = true;
    }

    // no colliders in the way; target is in sight
    return true;
}

void Game::random_target(int x, int y, int *next_x, int *next_y) const {
    if (next_x == NULL || next_y == NULL) return; // legit wtf if this happens

    x /= this->tile_width;
    y /= this->tile_height;
    
    std::vector<std::pair<int, int>> candidates;

    int start_x = clamp(x-1, 0, this->cols-1);
    int start_y = clamp(y-1, 0, this->rows-1);
    int end_x = clamp(x+1, 0, this->cols-1);
    int end_y = clamp(y+1, 0, this->rows-1);

    for (int _x = start_x; _x <= end_x; _x++) {
        for (int _y = start_y; _y <= end_y; _y++) {
            if (_x == x && _y == y) continue;

            if (this->collision[(_y * this->cols) + _x] < 0) {
                candidates.push_back({ _x, _y });
            }
        }
    }

    std::pair<int, int> target = candidates[SDL_rand(candidates.size())];
    *next_x = target.first * this->tile_width;
    *next_y = target.second * this->tile_height;
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

    if (this->collision != nullptr) {
        free(this->collision);
        this->collision = nullptr;
    }

    // clear ui
    this->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
}

void Game::load_map(const char *map_path) {
    Map map;
    map.read(map_path);

    for (auto obj : map.objects)
        this->create_object(obj.first, obj.second);

    this->bg = map.bg;
    this->fg = map.fg;
    this->collision = map.collision;
    this->tile_width = map.tile_width;
    this->tile_height = map.tile_height;
    this->cols = map.cols;
    this->rows = map.rows;

    // assemble appropriately sized quads for each predefined map collider
    for (int i = 0; i < n_MapColliders; i++) {
        // a quad has four vertices
        for (int j = 0; j < 4; j++) {
            this->colliders[i].vertex[j].x = float(map.tile_width) * MapColliders[i][j].x;
            this->colliders[i].vertex[j].y = float(map.tile_height) * MapColliders[i][j].y;
        }
    }

    map.clear();
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

void Game::save_objects() {
    for (auto obj : this->objects)
        obj->save_data();
}

void Game::post_save_objects() {
    for (auto obj : this->objects)
        obj->post_save_data();
}

void Game::set_view(int x, int y) {
    this->view_x = x;
    this->view_y = y;
    this->corner_x = x - SCREEN_WIDTH/2;
    this->corner_y = y - SCREEN_HEIGHT/2;
}

void Game::push_sprite(SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *dst_rect, int depth_offset) {
    int sprite_y = 0;
    if (dst_rect != nullptr)
        sprite_y = int(dst_rect->y) + depth_offset;

    SpriteRender sprite;
    sprite.texture = texture;
    sprite.src_rect = src_rect;
    sprite.dst_rect = dst_rect;
    sprite.y = sprite_y;

    // std::lower_bound performs a binary search (log n time complexity)
    auto it = std::lower_bound(this->sprites.begin(), this->sprites.end(), sprite);
    this->sprites.insert(it, sprite);
}

void Game::draw_rect(SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode) {
    SDL_SetRenderTarget(renderer, this->ui);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(renderer, blend_mode);
    SDL_RenderFillRect(renderer, rect);
    SDL_SetRenderTarget(renderer, this->screen);
}

void Game::draw_outline(SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode) {
    SDL_SetRenderTarget(renderer, this->ui);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(renderer, blend_mode);
    SDL_RenderRect(renderer, rect);
    SDL_SetRenderTarget(renderer, this->screen);
}

#define UI_BOX_SIZE 8.0f

void Game::draw_ui_box(SDL_FRect *rect) {
    SDL_FRect src_rect, dst_rect;
    SDL_SetRenderTarget(renderer, this->ui);

    // top-left corner
    src_rect = { 0.0f, 0.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x, rect->y, UI_BOX_SIZE, UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // top-right corner
    src_rect = { UI_BOX_SIZE * 2.0f, 0.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + rect->w - UI_BOX_SIZE, rect->y, UI_BOX_SIZE, UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // bottom-right corner
    src_rect = { UI_BOX_SIZE * 2.0f, UI_BOX_SIZE * 2.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + rect->w - UI_BOX_SIZE, rect->y + rect->h - UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // bottom-left corner
    src_rect = { 0.0f, UI_BOX_SIZE * 2.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x, rect->y + rect->h - UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // top edge
    src_rect = { UI_BOX_SIZE, 0.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + UI_BOX_SIZE, rect->y, rect->w - (UI_BOX_SIZE * 2.0f), UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // right edge
    src_rect = { UI_BOX_SIZE * 2.0f, UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + rect->w - UI_BOX_SIZE, rect->y + UI_BOX_SIZE, UI_BOX_SIZE, rect->h - (UI_BOX_SIZE * 2.0f) };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // bottom edge
    src_rect = { UI_BOX_SIZE, UI_BOX_SIZE * 2.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + UI_BOX_SIZE, rect->y + rect->h - UI_BOX_SIZE, rect->w - (UI_BOX_SIZE * 2.0f), UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // left edge
    src_rect = { 0.0f, UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x, rect->y + UI_BOX_SIZE, UI_BOX_SIZE, rect->h - (UI_BOX_SIZE * 2.0f) };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // middle
    src_rect = { UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + UI_BOX_SIZE, rect->y + UI_BOX_SIZE, rect->w - (UI_BOX_SIZE * 2.0f), rect->h - (UI_BOX_SIZE * 2.0f) };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    SDL_SetRenderTarget(renderer, this->screen);
}

void Game::draw_text(const std::string &str, int font, int x, int y, int w) {
    if (font < 0 || font >= NUM_FONTS) {
        std::cerr << "Game::draw_text error: '" << font << "' font does not exist" << std::endl;
        exit(1);
    }

    Font *_font = this->fonts[font];
    int _x = x, _y = y, line_height = int(_font->src_rect[0].h) + 1;
    int running_width = 0;
    SDL_FRect dst_rect;

    SDL_SetRenderTarget(renderer, this->ui);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    for (int i = 0; i < str.size(); i++) {
        char c = str[i];
        if (c >= ' ' && c <= '~') {
            SDL_FRect *src_rect = _font->src_rect + (c - ' ');

            if (w > 0) {
                int rendered_width = running_width;
                running_width += int(src_rect->w);
                
                // if running too wide, let's back-track and go to the next line
                if (running_width > w) {
                    int j = i;
                    // back-track to the last space
                    while (str[i] != ' ' && i > 0) {
                        c = str[i--];
                        // subtract the width of each rendered character from running_width
                        if (c >= ' ' && c <= '~') {
                            src_rect = _font->src_rect + (c - ' ');
                            running_width -= int(src_rect->w);
                        }
                    }
                    i++;

                    // undo the rendered characters
                    if (running_width < rendered_width) {
                        SDL_FRect undo_rect = { float(x + running_width), float(_y), float(rendered_width - running_width), float(line_height) };
                        SDL_RenderFillRect(renderer, &undo_rect);
                    }

                    // go to next line
                    _x = x;
                    _y += line_height;
                    running_width = 0;

                    // render the previously rendered characters
                    while (i < j) {
                        c = str[i++];
                        src_rect = _font->src_rect + (c - ' ');
                        running_width += int(src_rect->w);
                        dst_rect = { float(_x), float(_y), src_rect->w, src_rect->h };
                        _x += int(src_rect->w);
                        SDL_RenderTexture(renderer, _font->texture, src_rect, &dst_rect);
                    }

                    i--; // continue will increment i
                    continue;
                }
            }

            // render character
            dst_rect = { float(_x), float(_y), src_rect->w, src_rect->h };
            SDL_RenderTexture(renderer, _font->texture, src_rect, &dst_rect);
            _x += int(src_rect->w);
        } else {
            switch (c) {
            case '\n':
                _x = x;
                _y += line_height;
                running_width = 0;
                break;
            default: /* pass */ break;
            }
        }
    }

    SDL_SetRenderTarget(renderer, this->screen);
}

void Game::step() {
    if (this->map != "") {
        // clear the window
        SDL_SetRenderTarget(renderer, NULL);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        this->unload();
        this->load_map(this->map.c_str());
        this->map = "";
        return;
    }

    this->ticks = SDL_GetTicks();
    float delta2 = float(this->ticks - this->last_ticks) / 1000.0f;
    this->delta = (2.0f*this->delta + delta2) / 3.0f;
    this->last_ticks = this->ticks;

    if (this->ticks - this->frame_ticks > 20 * 1000) {
        std::cout << "Avg FPS: " << this->frames / 20 << std::endl;
        this->frames = 0;
        this->frame_ticks = this->ticks;
    } else this->frames++;

    // step all objects
    for (auto it = this->objects.begin(); it != this->objects.end();) {
        (*it)->step();

        if (this->delete_object) {
            delete *it;
            it = this->objects.erase(it);
            this->delete_object = false;
        } else it++;
    }

    SDL_SetRenderTarget(renderer, this->screen);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // render background
    SDL_FRect map_src, map_dst;
    if (this->bg != nullptr) {
        this->make_map_rect(this->view_x, this->view_y, this->bg->w, this->bg->h, &map_src, &map_dst);
        SDL_RenderTexture(renderer, this->bg, &map_src, &map_dst);
    }

    // make sure all sprites are blended, not replacing, pixels
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // render objects to the screen
    for (SpriteRender &sprite : this->sprites)
        SDL_RenderTexture(renderer, sprite.texture, sprite.src_rect, sprite.dst_rect);
    this->sprites.clear(); // clear sprites; next frame will repopulate

    // render foreground
    if (this->fg != nullptr)
        SDL_RenderTexture(renderer, this->fg, &map_src, &map_dst);

    // render ui
    SDL_RenderTexture(renderer, this->ui, NULL, NULL);

    if (keyboard.is_hit(SDLK_P)) {
        SDL_Surface *_screen = SDL_RenderReadPixels(renderer, NULL);
        SDL_SaveBMP(_screen, "screenshot.bmp");
        SDL_DestroySurface(_screen);
    }

    SDL_SetRenderTarget(renderer, NULL);
}
