#include "bmp_texture.h"
#include "game.h"
#include "map.h"

#include <SDL3/SDL.h>

#include <iostream>

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

#define UI_BOX_SIZE 4.0f

void Game::draw_ui_box(int type, SDL_FRect *rect) {
    SDL_SetRenderTarget(renderer, this->ui);

    int x_shift = type * (UI_BOX_SIZE * 3);
    SDL_FRect src_rect, dst_rect;

    // top-left corner
    src_rect = { float(x_shift), 0.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x, rect->y, UI_BOX_SIZE, UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // top-right corner
    src_rect = { float(x_shift + 2*UI_BOX_SIZE), 0.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + rect->w - UI_BOX_SIZE, rect->y, UI_BOX_SIZE, UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // bottom-right corner
    src_rect = { float(x_shift + 2*UI_BOX_SIZE), UI_BOX_SIZE * 2.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + rect->w - UI_BOX_SIZE, rect->y + rect->h - UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // bottom-left corner
    src_rect = { float(x_shift), UI_BOX_SIZE * 2.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x, rect->y + rect->h - UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    SDL_RenderTexture(renderer, this->ui_box, &src_rect, &dst_rect);

    // top edge
    src_rect = { float(x_shift + UI_BOX_SIZE), 0.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + UI_BOX_SIZE, rect->y, rect->w - (UI_BOX_SIZE * 2.0f), UI_BOX_SIZE };
    SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

    // right edge
    src_rect = { float(x_shift + 2*UI_BOX_SIZE), UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + rect->w - UI_BOX_SIZE, rect->y + UI_BOX_SIZE, UI_BOX_SIZE, rect->h - (UI_BOX_SIZE * 2.0f) };
    SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

    // bottom edge
    src_rect = { float(x_shift + UI_BOX_SIZE), UI_BOX_SIZE * 2.0f, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + UI_BOX_SIZE, rect->y + rect->h - UI_BOX_SIZE, rect->w - (UI_BOX_SIZE * 2.0f), UI_BOX_SIZE };
    SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

    // left edge
    src_rect = { float(x_shift), UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x, rect->y + UI_BOX_SIZE, UI_BOX_SIZE, rect->h - (UI_BOX_SIZE * 2.0f) };
    SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

    // middle
    src_rect = { float(x_shift + UI_BOX_SIZE), UI_BOX_SIZE, UI_BOX_SIZE, UI_BOX_SIZE };
    dst_rect = { rect->x + UI_BOX_SIZE, rect->y + UI_BOX_SIZE, rect->w - (UI_BOX_SIZE * 2.0f), rect->h - (UI_BOX_SIZE * 2.0f) };
    SDL_RenderTextureTiled(renderer, this->ui_box, &src_rect, 1.0f, &dst_rect);

    SDL_SetRenderTarget(renderer, this->screen);
}

void Game::draw_text(const std::string &str, int font, int x, int y, int w) {
    if (font < 0 || font >= NUM_FONTS) {
        std::cerr << "Game::draw_text error: '" << font << "' font does not exist" << std::endl;
        exit(1);
    }

    SDL_SetRenderTarget(renderer, this->ui);

    Font *_font = this->fonts[font];
    int _x = x, _y = y, line_height = int(_font->src_rect[0].h) + 1;
    int running_width = 0;
    SDL_FRect dst_rect;
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

void Game::draw_icon(SDL_FRect icon, SDL_FRect *dst_rect) {
    SDL_SetRenderTarget(renderer, this->ui);

    SDL_RenderTexture(renderer, this->icons, &icon, dst_rect);

    SDL_SetRenderTarget(renderer, this->screen);
}

SDL_FRect Game::draw_health_bar(int health, int max_health, int x, int y) {
    SDL_SetRenderTarget(renderer, this->ui);

    int w_mul = 2;
    if (max_health < 4) {
        w_mul = 3;
    }

    int health_w = (max_health*w_mul) + 2;
    int health_x = x - (health_w/2);
    int health_y = y - 5;

    SDL_FRect draw_rect = { float(health_x), float(health_y), float(health_w), 5.0f };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &draw_rect);
    draw_rect.w = float((health*w_mul) + 1);
    SDL_SetRenderDrawColor(renderer, 255, 128, 96, 255);
    SDL_RenderFillRect(renderer, &draw_rect);
    draw_rect.w = float(health_w);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &draw_rect);

    SDL_SetRenderTarget(renderer, this->screen);

    return draw_rect;
}

void Game::draw_hud(std::string item, int health, int max_health, int cheese) {
    // container ui box
    SDL_FRect hud_rect = { 276.0f, 184.0f, 32.0f, 44.0f };
    this->draw_ui_box(BOX_MENU_CONT, &hud_rect);

    // current item
    SDL_SetRenderTarget(renderer, this->ui);
    SDL_Texture *item_texture = load_bmp_texture("sprites/" + item + ".bmp");
    SDL_FRect item_rect = { 284.0f, 188.0f, 16.0f, 16.0f };
    SDL_RenderTexture(renderer, item_texture, NULL, &item_rect);
    SDL_SetRenderTarget(renderer, this->screen);

    // health
    char buff[256];
    snprintf(buff, sizeof(buff), "{%d/%d", health, max_health);
    this->draw_text(buff, SMALL_FONT, max_health > 9 && health > 9 ? 280 : 282, 206, 0);

    // cheese
    snprintf(buff, sizeof(buff), "~ %d", cheese);
    this->draw_text(buff, SMALL_FONT, 282, 214, 0);
}
