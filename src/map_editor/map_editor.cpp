#include "bmp_texture.h"
#include "font.h"
#include "game.h"
#include "controller.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

#include <cstdlib>
#include <iostream>
#include <vector>

enum EditorSelectedLayer {
    BACKGROUND,
    FOREGROUND,
    COLLISION
};

enum EditorInputMode {
    INPUT_TILE,
    INPUT_SHEET
};

class Editor : public Object {
public:
    Editor(const char *map_path);
    ~Editor();

    void step() override;

private:
    void input_tile();
    void input_sheet();
    void render();
    void render_collision_tile(SDL_Texture *target, int x, int y, int collider);
    void render_objects();

    SDL_Texture *texture = nullptr;

    std::string map_path;
    Map map;

    Quad collider_quads[n_MapColliders];
    SDL_Texture *collision_texture, *collision_sheet;

    EditorSelectedLayer layer = BACKGROUND;
    int sel_x = 0, sel_y = 0;
    int sel_tilesheet = -1, sel_ts_x = 0, sel_ts_y = 0, sel_ts2_x = -1, sel_ts2_y = -1;
    int sel_collider = -1;

    bool user_inputting = false;
    EditorInputMode input_mode;
    std::string text;

    SDL_Texture *objects;
};

class EditorFactory : public ObjectFactory {
public:
    EditorFactory() {}
    ~EditorFactory() {}

    Object *create(const std::string &options) override {
        return new Editor(options.c_str());
    }
};

void Game::init() {
    if (this->argc < 2) {
        _running = false;
        std::cerr << "Please provide a map to edit via the command line." << std::endl;
        return;
    }

    this->factories["editor"] = new EditorFactory();
    this->create_object("editor", std::string(this->argv[1]));

    this->title = "Map Editor - ";
    this->title += this->argv[1];
}

Editor::Editor(const char *map_path) {
    std::string bin_path = std::string(map_path) + ".bin";
    FILE *file = fopen(bin_path.c_str(), "rb");
    if (file == nullptr) {
        if (game->argc < 8) {
            std::cerr << "To create a new map please provide: <map name> <tw> <th> <cols> <rows> <fill(0/1)> <fillts(opt)>." << std::endl;
            _running = false;
            return;
        }

        int tile_width = atoi(game->argv[2]);
        int tile_height = atoi(game->argv[3]);
        int cols = atoi(game->argv[4]);
        int rows = atoi(game->argv[5]);
        bool should_fill = atoi(game->argv[6]);

        this->map.make_empty(tile_width, tile_height, cols, rows);

        if (should_fill) {
            Tilesheet *tilesheet = new Tilesheet(game->argv[7], tile_width, tile_height);
            if (tilesheet->texture == nullptr) {
                std::cerr << game->argv[6] << " is not a valid tilesheet." << std::endl;
                _running = false;
                return;
            }
            this->map.tilesheets.push_back(tilesheet);

            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                    std::vector<Tile> *tiles = new std::vector<Tile>();
                    unsigned int tilex = rand() % 4;
                    tiles->push_back(Tile{0, tilex, 0});
                    this->map.bg_tiles[(y * cols) + x] = tiles;
                    this->map.render_tile(x, y);
                }
            }
        }
    } else {
        fclose(file);
        this->map.read(map_path);
        std::string backup_path = map_path;
        backup_path += ".backup";
        this->map.write(backup_path.c_str());
    }

    this->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

    this->map_path = map_path;

    if (!this->map.tilesheets.empty())
        this->sel_tilesheet = 0;

    this->collision_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->map.bg->w, this->map.bg->h);

    int collision_sheet_w = this->map.tile_width * 14;
    int collision_sheet_h = this->map.tile_height * 3;
    this->collision_sheet = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, collision_sheet_w, collision_sheet_h);

    // assemble appropriately sized quads for each collider
    for (int i = 0; i < n_MapColliders; i++) {
        // a quad has four vertices
        for (int j = 0; j < 4; j++) {
            this->collider_quads[i].vertex[j].x = float(this->map.tile_width) * MapColliders[i][j].x;
            this->collider_quads[i].vertex[j].y = float(this->map.tile_height) * MapColliders[i][j].y;
        }
    }

    // render the collision texture
    for (int i = 0; i < this->map.cols * this->map.rows; i++) {
        int collider = this->map.collision[i];
        this->render_collision_tile(this->collision_texture, i % this->map.cols, i / this->map.cols, collider);
    }

    // render the collision sheet for the ease of the user
    // start at -1 for no collider, 0.. for colliders
    for (int i = -1; i < int(n_MapColliders); i++) {
        int x = (i+1) % 14;
        int y = (i+1) / 14;
        this->render_collision_tile(this->collision_sheet, x, y, i);
    }

    this->objects = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->map.bg->w, this->map.bg->h);
    SDL_SetTextureAlphaMod(this->objects, 0xa0);
    this->render_objects();

    this->sel_x = this->map.cols / 2;
    this->sel_y = this->map.rows / 2;
}

Editor::~Editor() {
    SDL_DestroyTexture(this->texture);
    if (this->map.bg != nullptr) {
        SDL_DestroyTexture(this->map.bg);
        this->map.bg = nullptr;
    }
    if (this->map.fg != nullptr) {
        SDL_DestroyTexture(this->map.fg);
        this->map.fg = nullptr;
    }
    if (this->map.collision != nullptr) {
        free(this->map.collision);
        this->map.collision = nullptr;
    }
    if (this->collision_texture != nullptr) {
        SDL_DestroyTexture(this->collision_texture);
        this->collision_texture = nullptr;
    }
    if (this->collision_sheet != nullptr) {
        SDL_DestroyTexture(this->collision_sheet);
        this->collision_sheet = nullptr;
    }
}

void Editor::step() {
    if (!this->user_inputting) {
        // move the selected tile on input
        this->sel_x += int(local_controller.is_hit(Button::RIGHT)) - int(local_controller.is_hit(Button::LEFT));
        this->sel_y += int(local_controller.is_hit(Button::DOWN)) - int(local_controller.is_hit(Button::UP));
        this->sel_x = cnf_clamp(this->sel_x, 0, this->map.cols - 1);
        this->sel_y = cnf_clamp(this->sel_y, 0, this->map.rows - 1);

        std::vector<Tile> *vec = nullptr;
        int coord = (this->sel_y * this->map.cols) + this->sel_x;
        switch (this->layer) {
        case BACKGROUND: vec = this->map.bg_tiles[coord]; break;
        case FOREGROUND: vec = this->map.fg_tiles[coord]; break;
        default: /* pass */ break;
        }

        SDL_FRect black_rect = { 0.0f, 0.0f, float(SCREEN_WIDTH), 11.0f };
        game->draw_rect(game->ui, &black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
        game->draw_text(game->ui, " h:write, j:tile, k:sheet, l:layer, x:export  ", MONO_FONT, 0, 0, 0);
        char status_text[256];
        if (this->layer != COLLISION) {
            // foreground / background status text
            snprintf(status_text, sizeof(status_text), "%s: %s %d,%d [%d]",
                this->map_path.c_str(),
                this->layer == BACKGROUND ? "bg" : "fg",
                this->sel_x * this->map.tile_width,
                this->sel_y * this->map.tile_height,
                vec == nullptr ? 0 : int(vec->size()));
        } else {
            // collision layer status text
            snprintf(status_text, sizeof(status_text), "%s: %s %d,%d",
                this->map_path.c_str(),
                "collision",
                this->sel_x * this->map.tile_width,
                this->sel_y * this->map.tile_height);
        }
        black_rect = { 0.0f, float(SCREEN_HEIGHT - 11), float(SCREEN_WIDTH), 11.0f };
        game->draw_rect(game->ui, &black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
        game->draw_text(game->ui, std::string(status_text), MONO_FONT, 0, SCREEN_HEIGHT - 11, 0);

        switch (local_controller.c) {
        case 'h':
            this->map.write(this->map_path.c_str());
            break;
        case 'j':
            if (this->sel_tilesheet < 0)
                break;

            this->user_inputting = true;
            this->input_mode = INPUT_TILE;
            this->text = "";
            break;
        case 'k':
            if (this->sel_tilesheet < 0)
                break;

            this->user_inputting = true;
            this->input_mode = INPUT_SHEET;
            this->text = "";
            break;
        case 'l':
            switch (this->layer) {
            case BACKGROUND: this->layer = FOREGROUND; break;
            case FOREGROUND: this->layer = COLLISION; break;
            default: this->layer = BACKGROUND; break;
            }
            break;
        case 'x': {
            std::cout << "exporting map to mapdump.bmp" << std::endl;
            local_controller.c = NO_CHAR;
            SDL_SetRenderTarget(renderer, this->map.bg);
            SDL_Surface *_bg = SDL_RenderReadPixels(renderer, NULL);
            SDL_SetRenderTarget(renderer, game->screen);
            SDL_SaveBMP(_bg, "mapdump.bmp");
            SDL_DestroySurface(_bg);
        } break;
        default: break;
        }
        local_controller.c = NO_CHAR;

        if (local_controller.is_hit(Button::ATTACK) && this->sel_tilesheet >= 0 && this->sel_ts2_x >= 0) {
            if (this->layer != COLLISION) {
                int _x = cnf_min(this->sel_ts_x, this->sel_ts2_x);
                int _y = cnf_min(this->sel_ts_y, this->sel_ts2_y);
                int _w = cnf_abs(this->sel_ts_x - this->sel_ts2_x);
                int _h = cnf_abs(this->sel_ts_y - this->sel_ts2_y);

                for (int dy = 0; dy <= _h; dy++) {
                    for (int dx = 0; dx <= _w; dx++) {
                        if (this->sel_x + dx >= this->map.cols) {
                            continue;
                        }

                        if (this->sel_y + dy >= this->map.rows) {
                            break;
                        }

                        int _coord = coord + dx + (this->map.cols * dy);
                        std::vector<Tile> *_vec;
                        if (this->layer == FOREGROUND) {
                            _vec = this->map.fg_tiles[_coord];
                            if (_vec == nullptr) {
                                _vec = new std::vector<Tile>();
                                this->map.fg_tiles[_coord] = _vec;
                            }
                        } else {
                            _vec = this->map.bg_tiles[_coord];
                            if (_vec == nullptr) {
                                _vec = new std::vector<Tile>();
                                this->map.bg_tiles[_coord] = _vec;
                            }
                        }
                        Tile tile;
                        tile.tilesheet = this->sel_tilesheet;
                        tile.x = _x + dx;
                        tile.y = _y + dy;
                        _vec->push_back(tile);
                        this->map.render_tile(this->sel_x + dx, this->sel_y + dy);
                    }
                }
            }
        } else if (local_controller.is_hit(Button::TOSS) && this->sel_tilesheet >= 0 && this->sel_ts2_x >= 0) {
            int _x = cnf_min(this->sel_ts_x, this->sel_ts2_x);
            int _y = cnf_min(this->sel_ts_y, this->sel_ts2_y);
            int _w = (cnf_abs(this->sel_ts_x - this->sel_ts2_x) + 1) * this->map.tile_width;
            int _h = (cnf_abs(this->sel_ts_y - this->sel_ts2_y) + 1) * this->map.tile_height;
            char buff[1024];
            snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d,%d,%d,%s",
                this->sel_x * this->map.tile_width,
                this->sel_y * this->map.tile_height,
                _x, _y, _w, _h, _h, this->map.tilesheets[this->sel_tilesheet]->path.c_str());
            this->map.objects.push_back(std::pair<std::string, std::string>("billboard", buff));
            this->render_objects();
        } else if (local_controller.is_down(Button::SELECT) && this->sel_tilesheet >= 0) {
            if (this->layer == COLLISION) {
                this->map.collision[coord] = this->sel_collider;
                this->render_collision_tile(this->collision_texture, this->sel_x, this->sel_y, this->sel_collider);
            } else {
                bool should_place = true;

                // place a tile
                if (vec == nullptr) {
                    vec = new std::vector<Tile>();
                    if (this->layer == FOREGROUND) {
                        this->map.fg_tiles[coord] = vec;
                    } else {
                        this->map.bg_tiles[coord] = vec;
                    }
                } else {
                    Tile top_tile = vec->back();
                    if (top_tile.tilesheet == this->sel_tilesheet && top_tile.x == this->sel_ts_x && top_tile.y == this->sel_ts_y)
                        should_place = false;
                }

                if (should_place) {
                    Tile new_tile;
                    new_tile.tilesheet = this->sel_tilesheet;
                    new_tile.x = this->sel_ts_x;
                    new_tile.y = this->sel_ts_y;
                    vec->push_back(new_tile);
                    this->map.render_tile(this->sel_x, this->sel_y);
                }
            }
        } else if (local_controller.is_hit(Button::CANCEL)) {
            if (this->layer == COLLISION) {
                this->map.collision[coord] = -1;
                this->render_collision_tile(this->collision_texture, this->sel_x, this->sel_y, -1);
            } else {
                // remove a tile
                if (vec != nullptr) {
                    vec->pop_back();
                    if (vec->empty()) {
                        delete vec;
                        vec = nullptr;
                        if (this->layer == FOREGROUND) {
                            this->map.fg_tiles[coord] = nullptr;
                        } else {
                            this->map.bg_tiles[coord] = nullptr;
                        }
                    }
                    this->map.render_tile(this->sel_x, this->sel_y);
                }
            }
        }
    }

    this->render();

    if (this->user_inputting) {
        switch (this->input_mode) {
        case INPUT_TILE:
            this->input_tile();
            break;
        case INPUT_SHEET:
            this->input_sheet();
            break;
        default:
            this->user_inputting = false;
            game->draw_rect(game->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            break;
        }
    }

    game->push_sprite("EDITOR", this->texture, NULL, NULL, 0);
}

void Editor::input_tile() {
    // end inputting
    if (local_controller.is_hit(Button::SELECT) || local_controller.is_hit(DIGIT)) {
        this->user_inputting = false;
        game->draw_rect(game->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        return;
    }

    // update selected collider or tile
    if (this->layer == COLLISION) {
        this->sel_collider += int(local_controller.is_hit(Button::RIGHT)) - int(local_controller.is_hit(Button::LEFT));
        int y_dir = int(local_controller.is_hit(Button::DOWN)) - int(local_controller.is_hit(Button::UP));
        this->sel_collider += y_dir * 14;

        this->sel_collider = cnf_clamp(this->sel_collider+1, 0, n_MapColliders) - 1;
    } else {    
        this->sel_ts_x += int(local_controller.is_hit(Button::RIGHT)) - int(local_controller.is_hit(Button::LEFT));
        this->sel_ts_y += int(local_controller.is_hit(Button::DOWN)) - int(local_controller.is_hit(Button::UP));
        this->sel_ts_x = cnf_clamp(this->sel_ts_x, 0, this->map.tilesheets[this->sel_tilesheet]->cols - 1);
        this->sel_ts_y = cnf_clamp(this->sel_ts_y, 0, this->map.tilesheets[this->sel_tilesheet]->rows - 1);

        if (local_controller.is_hit(Button::ATTACK)) { // space
            this->sel_ts2_x = this->sel_ts_x;
            this->sel_ts2_y = this->sel_ts_y;
        } else if (local_controller.is_hit(Button::TOSS)) { // c
            this->sel_ts2_x = this->sel_ts2_y = -1;
        }
    }

    SDL_SetRenderTarget(renderer, this->texture);
    SDL_FRect tilesheet_rect;
    tilesheet_rect.x = 0.0f;
    tilesheet_rect.y = 11.0f;

    if (this->layer == COLLISION) {
        tilesheet_rect.w = this->collision_sheet->w;
        tilesheet_rect.h = this->collision_sheet->h;
    } else {
        tilesheet_rect.w = this->map.tilesheets[this->sel_tilesheet]->texture->w;
        tilesheet_rect.h = this->map.tilesheets[this->sel_tilesheet]->texture->h;
    }

    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &tilesheet_rect);

    if (this->layer == COLLISION) {
        SDL_RenderTexture(renderer, this->collision_sheet, NULL, &tilesheet_rect);
    } else {
        SDL_RenderTexture(renderer, this->map.tilesheets[this->sel_tilesheet]->texture, NULL, &tilesheet_rect);
    }

    SDL_FRect sel_tile_rect;
    sel_tile_rect.w = this->map.tile_width;
    sel_tile_rect.h = this->map.tile_height;

    if (this->layer == COLLISION) {
        int _x = (this->sel_collider+1) % 14;
        int _y = (this->sel_collider+1) / 14;
        sel_tile_rect.x = this->map.tile_width * _x;
        sel_tile_rect.y = tilesheet_rect.y + (this->map.tile_height * _y);
    } else {
        sel_tile_rect.x = this->map.tile_width * this->sel_ts_x;
        sel_tile_rect.y = tilesheet_rect.y + this->map.tile_height * this->sel_ts_y;
    }

    SDL_SetRenderDrawColor(renderer, 255, 128, 255, 240);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderRect(renderer, &sel_tile_rect);

    if (this->sel_ts2_x >= 0) {
        SDL_FRect sel_tile2_rect;
        sel_tile2_rect.w = this->map.tile_width;
        sel_tile2_rect.h = this->map.tile_height;
        sel_tile2_rect.x = this->map.tile_width * this->sel_ts2_x;
        sel_tile2_rect.y = tilesheet_rect.y + this->map.tile_height * this->sel_ts2_y;

        SDL_SetRenderDrawColor(renderer, 255, 255, 128, 240);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderRect(renderer, &sel_tile2_rect);
    }

    SDL_SetRenderTarget(renderer, game->screen);
    SDL_FRect black_rect = { 0.0f, 0.0f, float(SCREEN_WIDTH), 11.0f };
    game->draw_rect(game->ui, &black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
    game->draw_text(game->ui, "<Return> to place, <Q> to close", MONO_FONT, 0, 0, 0);
}

void Editor::input_sheet() {
    int sel_i, nfields;
    nfields = sscanf(this->text.c_str(), "%d", &sel_i);

    if (local_controller.is_hit(Button::SELECT) && text.empty()) {
        // close
        this->user_inputting = false;
        game->draw_rect(game->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        return;
    }

    if (local_controller.is_hit(Button::CANCEL) && !text.empty())
        this->text.pop_back();
    else if (local_controller.c != NO_CHAR)
        this->text.push_back(local_controller.c);
    else if (local_controller.is_hit(Button::SELECT) && nfields == 1) {
        this->sel_tilesheet = sel_i;
    }

    std::string display_text = "Tilesheet: " + this->text + "\n<Return> to select or close (when empty)\n\n";
    for (int i = 0; i < this->map.tilesheets.size(); i++) {
        char tilesheet[256];
        char first_c, second_c, end_char = i == this->map.tilesheets.size()-1 ? ' ' : '\n';

        if (nfields == 1 && i == sel_i) first_c = '*'; else first_c = ' ';
        if (i == this->sel_tilesheet) second_c = '*'; else second_c = ' ';

        snprintf(tilesheet, 256, "%c%c%d: %s%c", first_c, second_c, i, this->map.tilesheets[i]->path.c_str(), end_char);
        display_text += tilesheet;
    }
    SDL_FRect black_rect = { 0.0f, 0.0f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT) };
    game->draw_rect(game->ui, &black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
    game->draw_text(game->ui, display_text, MONO_FONT, 0, 0, 0);
}

void Editor::render() {
    SDL_FRect src, dst;

    int x = (this->sel_x*this->map.tile_width) + (this->map.tile_width/2);
    int y = (this->sel_y*this->map.tile_height) + (this->map.tile_height/2);

    game->make_map_rect(x - SCREEN_WIDTH/2, y - SCREEN_HEIGHT/2, this->map.tile_width * this->map.cols, this->map.tile_height * this->map.rows, &src, &dst);

    SDL_FRect selected_tile_rect;
    selected_tile_rect.x = (float)(int)((SCREEN_WIDTH/2) - (this->map.tile_width/2));
    selected_tile_rect.y = (float)(int)((SCREEN_HEIGHT/2) - (this->map.tile_height/2));
    selected_tile_rect.w = float(this->map.tile_width);
    selected_tile_rect.h = float(this->map.tile_height);

    SDL_SetRenderTarget(renderer, this->texture);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, this->map.bg, &src, &dst);

    // only render the foreground if not only background
    if (this->layer != BACKGROUND)
        SDL_RenderTexture(renderer, this->map.fg, &src, &dst);
    else {
        // render foreground at half opacity if viewing background
        SDL_SetTextureAlphaMod(this->map.fg, 0x80);
        SDL_RenderTexture(renderer, this->map.fg, &src, &dst);
        SDL_SetTextureAlphaMod(this->map.fg, 0xff);
    }

    // render collision layer if that mode is selected
    if (this->layer == COLLISION) {
        SDL_RenderTexture(renderer, objects, &src, &dst);
        SDL_RenderTexture(renderer, this->collision_texture, &src, &dst);
    }

    SDL_SetRenderDrawColor(renderer, 255, 128, 255, 240);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderRect(renderer, &selected_tile_rect);

    if (this->sel_tilesheet >= 0) {
        selected_tile_rect.x = 0.0f;
        selected_tile_rect.y = float(SCREEN_HEIGHT - 11 - this->map.tile_height);
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderFillRect(renderer, &selected_tile_rect);
        SDL_FRect selected_tile_src_rect;
        selected_tile_src_rect.w = float(this->map.tile_width);
        selected_tile_src_rect.h = float(this->map.tile_height);

        if (this->layer == COLLISION) {
            int _x = (this->sel_collider+1) % 14;
            int _y = (this->sel_collider+1) / 14;
            selected_tile_src_rect.x = float(_x * this->map.tile_width);
            selected_tile_src_rect.y = float(_y * this->map.tile_height);
            SDL_RenderTexture(renderer, this->collision_sheet, &selected_tile_src_rect, &selected_tile_rect);
        } else {
            selected_tile_src_rect.x = float(this->sel_ts_x * this->map.tile_width);
            selected_tile_src_rect.y = float(this->sel_ts_y * this->map.tile_height);
            SDL_RenderTexture(renderer, this->map.tilesheets[this->sel_tilesheet]->texture, &selected_tile_src_rect, &selected_tile_rect);
        }
    }

    SDL_SetRenderTarget(renderer, game->screen);
}

void Editor::render_collision_tile(SDL_Texture *target, int x, int y, int collider) {
    SDL_FRect dst_rect;
    dst_rect.x = float(this->map.tile_width * x);
    dst_rect.y = float(this->map.tile_height * y);
    dst_rect.w = float(this->map.tile_width);
    dst_rect.h = float(this->map.tile_height);

    // render transparency to clear the tile
    SDL_SetRenderTarget(renderer, target);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(renderer, &dst_rect);

    if (collider < 0) {
        SDL_SetRenderTarget(renderer, game->screen);
        return; // all done
    }

    // render the collision quad
    SDL_Vertex vertices[4];
    // translate the vertices to match the destination rect
    for (int i = 0; i < 4; i++) {
        vertices[i].color.r = 1.0f;
        vertices[i].color.g = 1.0f;
        vertices[i].color.b = 1.0f;
        vertices[i].color.a = 0.5f;
        vertices[i].position.x = dst_rect.x + this->collider_quads[collider].vertex[i].x;
        vertices[i].position.y = dst_rect.y + this->collider_quads[collider].vertex[i].y;
    }
    const int indices[6] = { 0, 1, 2, 2, 3, 0 };
    if (!SDL_RenderGeometry(renderer, nullptr, vertices, 4, indices, 6)) {
        std::cerr << "SDL_RenderGeometry error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_SetRenderTarget(renderer, game->screen);
}

static void _ParseBillboardOptions(const std::string &options, int *x, int *y, int *ts_x, int *ts_y, int *w, int *h, int *depth, std::string &tilesheet) {
    char buff[1024];
    if (sscanf(options.c_str(), "%d,%d,%d,%d,%d,%d,%d,%1023[^\n]",
        x, y, ts_x, ts_y, w, h, depth, buff) < 8) {
        std::cerr << "_ParseBillboardOptions error: bad options: " << options << std::endl;
        std::exit(1);
    }

    tilesheet = buff;
}

void Editor::render_objects() {
    SDL_SetRenderTarget(renderer, this->objects);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for (const std::pair<std::string, std::string> &pair : this->map.objects) {
        if (pair.first == "billboard") {
            int x, y, ts_x, ts_y, w, h, depth;
            std::string tilesheet;

            _ParseBillboardOptions(pair.second, &x, &y, &ts_x, &ts_y, &w, &h, &depth, tilesheet);

            SDL_Texture *tex = load_bmp_texture(tilesheet);

            SDL_FRect src_rect;
            src_rect.x = ts_x * this->map.tile_width;
            src_rect.y = ts_y * this->map.tile_height;
            src_rect.w = w;
            src_rect.h = h;
            SDL_FRect dst_rect = SDL_FRect{ float(x), float(y), float(w), float(h) };

            SDL_SetTextureAlphaMod(tex, 0x40);
            SDL_RenderTexture(renderer, tex, &src_rect, &dst_rect);
            SDL_SetTextureAlphaMod(tex, 0xff);

            SDL_SetRenderDrawColor(renderer, 255, 128, 0, 255);
            SDL_RenderRect(renderer, &dst_rect);
        } else {
            int x, y;
            if (sscanf(pair.second.c_str(), "%d,%d", &x, &y) == 2) {
                SDL_FRect dst_rect = SDL_FRect{ float(x) - 8.0f, float(y) - 8.0f, 16.0f, 16.0f };
                game->draw_icon(this->objects, EDITOR_OBJECT_ICON, &dst_rect);
                game->draw_text(this->objects, pair.first, SMALL_FONT, x + 6, y - 4, 0);
                SDL_SetRenderTarget(renderer, this->objects);
            }
        }
    }

    SDL_SetRenderTarget(renderer, game->screen);
}
