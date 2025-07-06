#include "SDL3/SDL_blendmode.h"
#include "bmp_texture.h"
#include "font.h"
#include "game.h"
#include "map.h"

#include <SDL3/SDL.h>

#include <iostream>

void Game::draw_rect(SDL_Texture *texture, SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode) {
    SDL_SetRenderTarget(renderer, texture);

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(renderer, blend_mode);
    SDL_RenderFillRect(renderer, rect);

    SDL_SetRenderTarget(renderer, this->screen);
}

void Game::draw_outline(SDL_Texture *texture, SDL_FRect *rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_BlendMode blend_mode) {
    SDL_SetRenderTarget(renderer, texture);

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(renderer, blend_mode);
    SDL_RenderRect(renderer, rect);

    SDL_SetRenderTarget(renderer, this->screen);
}

#define UI_BOX_SIZE 4.0f

void Game::draw_ui_box(SDL_Texture *texture, int type, SDL_FRect *rect) {
    SDL_SetRenderTarget(renderer, texture);

    SDL_FRect _rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    if (rect == NULL) {
        rect = &_rect;
    }

    int x_shift = type * ((int)UI_BOX_SIZE * 3);
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

void Game::draw_text(SDL_Texture *texture, std::string str, int font, int x, int y, int w) {
    if (font < 0 || font >= NUM_FONTS) {
        std::cerr << "Game::draw_text error: '" << font << "' font does not exist" << std::endl;
        exit(1);
    }

    SDL_SetRenderTarget(renderer, texture);

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

void Game::draw_icon(SDL_Texture *texture, SDL_FRect icon, SDL_FRect *dst_rect) {
    SDL_SetRenderTarget(renderer, texture);

    SDL_RenderTexture(renderer, this->icons, &icon, dst_rect);

    SDL_SetRenderTarget(renderer, this->screen);
}

void Game::draw_hud(SDL_Texture *texture, std::string item, int item_count, int health, int max_health, int cheese) {
    if (this->displaying_load_screen)
        return;

    // container ui box
    SDL_FRect hud_rect = { 276.0f, 184.0f, 32.0f, 44.0f };
    this->draw_ui_box(this->ui, BOX_MENU_CONT, &hud_rect);

    // current item
    SDL_FRect item_shadow = { 280.0f, 188.0f, 24.0f, 16.0f };
    this->draw_ui_box(texture, BOX_MENU_SHD1, &item_shadow);
    SDL_SetRenderTarget(renderer, texture);
    SDL_Texture *item_texture = load_bmp_texture("sprites/" + item + ".bmp");
    SDL_FRect item_src_rect = { 0.0f, 0.0f, 16.0f, 16.0f };
    SDL_FRect item_rect = { 284.0f, 188.0f, 16.0f, 16.0f };
    SDL_RenderTexture(renderer, item_texture, &item_src_rect, &item_rect);
    SDL_SetRenderTarget(renderer, this->screen);

    if (item_count > 1) {
        this->draw_icon(texture, ITEM_COUNT_ICON, &this->item_count_icon);
        char buff[256];
        snprintf(buff, sizeof(buff), "%d", item_count);
        this->draw_text(texture, buff, SMALL_FONT, item_count > 9 ? 298 : 300, 186, 0);
    }

    // health
    char buff[256];
    snprintf(buff, sizeof(buff), "{%d/%d", health, max_health);
    this->draw_text(texture, buff, SMALL_FONT, max_health > 9 && health > 9 ? 280 : 282, 204, 0);
    SDL_FRect health_rect = { 280.0f, 212.0f, 24.0f, 5.0f };
    float perc = ceil(22.0f * (float)health / (float)max_health);
    SDL_FRect fill_rect = { 281.0f, 213.0f, perc, 3.0f };
    this->draw_rect(texture, &health_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
    this->draw_rect(texture, &fill_rect, 255, 64, 64, 255, SDL_BLENDMODE_NONE);

    // cheese
    snprintf(buff, sizeof(buff), "~ %d", cheese);
    this->draw_text(texture, buff, SMALL_FONT, 282, 218, 0);
}

// static SDL_FRect _title_rect = SDL_FRect{ 0.0f, 20.0f, 320.0f, 40.0f };
static NetworkAgent::State _last_state = NetworkAgent::State::NO_CONNECTION;
static std::string _last_code = "";
static Uint64 _last_drawn_ticks = 0;
static SDL_FRect _netagent_rect = SDL_FRect{ 0.0f, 0.0f, 0.0f, 0.0f };
static SDL_FRect _neticon_rect = SDL_FRect{ 10.0f, 8.0f, 16.0f, 16.0f };
bool _displaying_netagent = false;
bool _displaying_notification = false;
bool _second_pass = false;

void Game::draw_overlay() {
    if (!this->display_overlay)
        return;

    SDL_SetRenderTarget(renderer, this->overlay);

    // network agent overlay (only cheddar can "create objects" so that is used to check if cheddar)
    if (game->create_objects && net_agent != nullptr) {
        this->net_state = net_agent->get_state();
        std::string code = net_agent->get_connection_code();

        // draw if state changed, code changed, or if a second has passed since last rendered
        if (this->net_state != _last_state || code != _last_code || this->ticks - _last_drawn_ticks > 1000) {
            // clear last overlay
            if (_netagent_rect.w > 0.0f) {
                this->draw_rect(this->overlay, &_netagent_rect, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            }

            switch (this->net_state) {
            case NetworkAgent::State::NO_CONNECTION:
                _netagent_rect = SDL_FRect{ 8.0f, 8.0f, 88.0f, 16.0f };
                this->draw_ui_box(this->overlay, BOX_MENU_CONT, &_netagent_rect);
                this->draw_icon(this->overlay, NOT_CONNECTED_ICON, &_neticon_rect);
                this->draw_text(this->overlay, "Not Connected", DEFAULT_FONT, 28, 12, 0);
                _displaying_netagent = true;
                break;
            case NetworkAgent::State::WAITING_FOR_PEER:
                _netagent_rect = SDL_FRect{ 8.0f, 8.0f, 80.0f, 16.0f };
                this->draw_ui_box(this->overlay, BOX_MENU_CONT, &_netagent_rect);
                this->draw_icon(this->overlay, WAITING_FOR_PEER_ICON, &_neticon_rect);
                this->draw_text(this->overlay, "Code:", SMALL_FONT, 28, 13, 0);
                this->draw_text(this->overlay, code, CODE_FONT, 48, 12, 0);
                _displaying_netagent = true;
                break;
            default:
                // clear entire overlay screen
                this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
                _netagent_rect = SDL_FRect{ 0.0f, 0.0f, 0.0f, 0.0f }; // no dst rect
                _displaying_netagent = false;
                _displaying_notification = false;
                break; // display nothing
            }

            _last_state = this->net_state;
            _last_code = code;
            _last_drawn_ticks = this->ticks;
        }
    }

    if (!this->notification.empty()) {
        if (this->force_notif_rerender) {
            this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            _displaying_notification = false;
            this->force_notif_rerender = false;

            if (_displaying_netagent && !_second_pass) {
                _last_drawn_ticks = 0;
                _second_pass = true;
                this->draw_overlay();
                _second_pass = false;
            }
        }

        if (!_displaying_notification) {
            int text_w = fonts[DEFAULT_FONT]->text_width(this->notification);
            SDL_FRect notif_rect = SDL_FRect{ 8.0f, 8.0f, (float)(text_w + 12), 16.0f };
            int text_y = 12;

            if (_displaying_netagent) {
                notif_rect.y = 26.0f;
                text_y = 30;
            }

            this->draw_ui_box(this->overlay, BOX_MENU_CONT, &notif_rect);
            this->draw_text(this->overlay, this->notification, DEFAULT_FONT, 14, text_y, 0);

            _displaying_notification = true;
        } else if (this->ticks - this->notif_ticks > 3000) {
            this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            this->notification.clear();
            _displaying_notification = false;

            if (_displaying_netagent && !_second_pass) {
                _last_drawn_ticks = 0;
                _second_pass = true;
                this->draw_overlay();
                _second_pass = false;
            }
        }
    }

    SDL_SetRenderTarget(renderer, this->screen);
}
