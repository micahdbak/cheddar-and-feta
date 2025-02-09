#include "game.h"
#include "keyboard.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

#include <iostream>
#include <vector>

enum EditorSelectedLayer {
    BACKGROUND,
    FOREGROUND,
    COLLISION
};

enum EditorInputMode {
    INPUT_TILE,
    INPUT_SHEET,
    INPUT_ADD_SHEET,
    INPUT_OBJECTS
};

class Editor : public Object {
public:
    Editor(const char *map_path);
    ~Editor();

    void step() override;

private:
    void input_tile();
    void input_sheet();
    void input_add_sheet();
    void input_objects();
    void render();
    void render_collision_tile(SDL_Texture *target, int x, int y, int collider);

    SDL_Texture *texture = nullptr;

    std::string map_path;
    Map map;

    Quad collider_quads[n_MapColliders];
    SDL_Texture *collision_texture, *collision_sheet;

    EditorSelectedLayer layer = BACKGROUND;
    int sel_x = 0, sel_y = 0;
    int sel_tilesheet = -1, sel_ts_x = 0, sel_ts_y = 0;
    int sel_collider = -1;

    bool user_inputting = false;
    EditorInputMode input_mode;
    std::string text;
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
    FILE *file = fopen(map_path, "rb");
    if (file == nullptr) {
        if (game->argc < 6) {
            std::cerr << "To create a new map please provide: <map name> <tile width> <tile height> <columns> <rows>." << std::endl;
            _running = false;
            return;
        }

        this->map.make_empty(atoi(game->argv[2]), atoi(game->argv[3]), atoi(game->argv[4]), atoi(game->argv[5]));
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
    this->collision_sheet = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->map.tile_width * (n_MapColliders+1), this->map.tile_height);

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
        this->render_collision_tile(this->collision_sheet, i+1, 0, i);
    }
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
        this->sel_x += int(keyboard.is_hit(SDLK_RIGHT)) - int(keyboard.is_hit(SDLK_LEFT));
        this->sel_y += int(keyboard.is_hit(SDLK_DOWN)) - int(keyboard.is_hit(SDLK_UP));
        this->sel_x = clamp(this->sel_x, 0, this->map.cols - 1);
        this->sel_y = clamp(this->sel_y, 0, this->map.rows - 1);

        std::vector<Tile> *vec = nullptr;
        int coord = (this->sel_y * this->map.cols) + this->sel_x;
        switch (this->layer) {
        case BACKGROUND: vec = this->map.bg_tiles[coord]; break;
        case FOREGROUND: vec = this->map.fg_tiles[coord]; break;
        default: /* pass */ break;
        }

        SDL_FRect black_rect = { 0.0f, 0.0f, float(SCREEN_WIDTH), 11.0f };
        game->draw_rect(&black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
        game->draw_text(" [T]ile, [S]heet, [A]dd, [L]ayer, [W]rite, [O]bjects  ", MONO_FONT, 0, 0, 0);
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
        game->draw_rect(&black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
        game->draw_text(std::string(status_text), MONO_FONT, 0, SCREEN_HEIGHT - 11, 0);

        switch (keyboard.c) {
        case 't':
            if (this->sel_tilesheet < 0)
                break;

            this->user_inputting = true;
            this->input_mode = INPUT_TILE;
            this->text = "";
            break;
        case 's':
            if (this->sel_tilesheet < 0)
                break;

            this->user_inputting = true;
            this->input_mode = INPUT_SHEET;
            this->text = "";
            break;
        case 'a':
            this->user_inputting = true;
            this->input_mode = INPUT_ADD_SHEET;
            this->text = "";
            break;
        case 'l':
            switch (this->layer) {
            case BACKGROUND: this->layer = FOREGROUND; break;
            case FOREGROUND: this->layer = COLLISION; break;
            default: this->layer = BACKGROUND; break;
            }
            break;
        case 'w':
            this->map.write(this->map_path.c_str());
            break;
        case 'o':
            this->user_inputting = true;
            this->input_mode = INPUT_OBJECTS;
            this->text = "";
            break;
        }
        keyboard.c = NO_CHAR;

        if (keyboard.is_down(SDLK_RETURN) && this->sel_tilesheet >= 0) {
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
                    // shift return should only place on empty tiles
                    if (keyboard.is_down(SDLK_RSHIFT))
                        should_place = false;

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
        } else if (keyboard.is_hit(SDLK_BACKSPACE) || keyboard.is_down(SDLK_MINUS)) {
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
        case INPUT_ADD_SHEET:
            this->input_add_sheet();
            break;
        case INPUT_OBJECTS:
            this->input_objects();
            break;
        default:
            this->user_inputting = false;
            game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            break;
        }
    }

    game->push_sprite(this->texture, NULL, NULL, 0);
}

void Editor::input_tile() {
    // end inputting
    if (keyboard.is_hit(SDLK_RETURN) || keyboard.is_hit(SDLK_ESCAPE)) {
        this->user_inputting = false;
        game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        return;
    }

    // update selected collider or tile
    if (this->layer == COLLISION) {
        this->sel_collider += int(keyboard.is_hit(SDLK_RIGHT)) - int(keyboard.is_hit(SDLK_LEFT));
        this->sel_collider = clamp(this->sel_collider+1, 0, n_MapColliders) - 1;
    } else {    
        this->sel_ts_x += int(keyboard.is_hit(SDLK_RIGHT)) - int(keyboard.is_hit(SDLK_LEFT));
        this->sel_ts_y += int(keyboard.is_hit(SDLK_DOWN)) - int(keyboard.is_hit(SDLK_UP));
        this->sel_ts_x = clamp(this->sel_ts_x, 0, this->map.tilesheets[this->sel_tilesheet]->cols - 1);
        this->sel_ts_y = clamp(this->sel_ts_y, 0, this->map.tilesheets[this->sel_tilesheet]->rows - 1);
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
        sel_tile_rect.x = this->map.tile_width * (this->sel_collider+1);
        sel_tile_rect.y = tilesheet_rect.y;
    } else {
        sel_tile_rect.x = this->map.tile_width * this->sel_ts_x;
        sel_tile_rect.y = tilesheet_rect.y + this->map.tile_height * this->sel_ts_y;
    }

    SDL_SetRenderDrawColor(renderer, 255, 128, 255, 240);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderRect(renderer, &sel_tile_rect);
    SDL_SetRenderTarget(renderer, game->screen);

    SDL_FRect black_rect = { 0.0f, 0.0f, float(SCREEN_WIDTH), 11.0f };
    game->draw_rect(&black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
    game->draw_text("<Return> to place, <Escape> to close", MONO_FONT, 0, 0, 0);
}

void Editor::input_sheet() {
    int sel_i, nfields;
    nfields = sscanf(this->text.c_str(), "%d", &sel_i);
    
    if (keyboard.is_hit(SDLK_BACKSPACE) && !text.empty())
        this->text.pop_back();
    else if (keyboard.c != NO_CHAR)
        this->text.push_back(keyboard.c);
    else if (keyboard.is_hit(SDLK_RETURN) && nfields == 1) {
        this->sel_tilesheet = sel_i;
    } else if (keyboard.is_hit(SDLK_ESCAPE)) {
        // close
        this->user_inputting = false;
        game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        return;
    }

    std::string display_text = "Tilesheet: " + this->text + "\n<Return> to select, <Escape> to close\n\n";
    for (int i = 0; i < this->map.tilesheets.size(); i++) {
        char tilesheet[256];
        char first_c, second_c, end_char = i == this->map.tilesheets.size()-1 ? ' ' : '\n';

        if (nfields == 1 && i == sel_i) first_c = '*'; else first_c = ' ';
        if (i == this->sel_tilesheet) second_c = '*'; else second_c = ' ';

        snprintf(tilesheet, 256, "%c%c%d: %s%c", first_c, second_c, i, this->map.tilesheets[i]->path.c_str(), end_char);
        display_text += tilesheet;
    }
    SDL_FRect black_rect = { 0.0f, 0.0f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT) };
    game->draw_rect(&black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
    game->draw_text(display_text, MONO_FONT, 0, 0, 0);
}

void Editor::input_add_sheet() {
    if (keyboard.is_hit(SDLK_BACKSPACE) && !text.empty())
        this->text.pop_back();
    else if (keyboard.c != NO_CHAR)
        this->text.push_back(keyboard.c);
    else if (keyboard.is_hit(SDLK_RETURN)) {
        // add new tilesheet
        Tilesheet *tilesheet = new Tilesheet(this->text.c_str(), this->map.tile_width, this->map.tile_height);
        if (tilesheet->texture != nullptr) {
            this->map.tilesheets.push_back(tilesheet);
            this->sel_tilesheet = this->map.tilesheets.size() - 1;
        } else {
            delete tilesheet;
        }
    } else if (keyboard.is_hit(SDLK_ESCAPE)) {
        // close
        this->user_inputting = false;
        game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        return;
    }

    SDL_FRect black_rect = { 0.0f, 0.0f, float(SCREEN_WIDTH), 22.0f };
    game->draw_rect(&black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
    game->draw_text("Tilesheet path: " + this->text + "\n<Return> to add, <Escape> to close", MONO_FONT, 0, 0, 0);
}

void Editor::input_objects() {
    int sel_i, nfields;
    char c, str[256];
    str[0] = '\0';
    nfields = sscanf(this->text.c_str(), "%d %c %[^\0]", &sel_i, &c, str);

    if (keyboard.is_hit(SDLK_BACKSPACE) && !text.empty())
        this->text.pop_back();
    else if (keyboard.c != NO_CHAR)
        this->text.push_back(keyboard.c);
    else if (keyboard.is_hit(SDLK_RETURN) && nfields >= 2) {
        if ((sel_i < 0 || sel_i >= this->map.objects.size()) && c == 'i') {
            // new object
            std::pair<std::string, std::string> obj;
            obj.first = str;
            obj.second = "";
            this->map.objects.push_back(obj);
        } else if (c == 'i') {
            // change id of an existing object
            this->map.objects[sel_i].first = str;
        } else if (c == 'o') {
            // change options for an existing object
            this->map.objects[sel_i].second = str;
        } else if (c == 'x') {
            // delete an object from the list
            auto it = this->map.objects.begin();
            std::advance(it, sel_i);
            this->map.objects.erase(it);
        }
        this->text = "";
    } else if (keyboard.is_hit(SDLK_ESCAPE)) {
        // leave menu
        this->user_inputting = false;
        game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        return;
    }

    std::string display_text = "<id> <i|o|x> <str>: " + this->text + "\n<Return> to enter, <Escape> to leave\n\n";
    for (int i = 0; i < this->map.objects.size(); i++) {
        std::pair<std::string, std::string> &obj = this->map.objects[i];

        char object[256];
        char start_char = ' ', end_char = i == this->map.objects.size()-1 ? ' ' : '\n';
        if (nfields >= 1 && i == sel_i)
            start_char = '*';

        snprintf(object, 256, "%c %d: %s %s%c", start_char, i, obj.first.c_str(), obj.second.c_str(), end_char);
        display_text += object;
    }
    SDL_FRect black_rect = { 0.0f, 0.0f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT) };
    game->draw_rect(&black_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);
    game->draw_text(display_text, MONO_FONT, 0, 0, 0);
}

void Editor::render() {
    SDL_FRect src, dst;

    int x = (this->sel_x*this->map.tile_width) + (this->map.tile_width/2);
    int y = (this->sel_y*this->map.tile_height) + (this->map.tile_height/2);

    game->make_map_rect(x - SCREEN_WIDTH/2, y - SCREEN_HEIGHT/2, this->map.tile_width * this->map.cols, this->map.tile_height * this->map.rows, &src, &dst);

    SDL_FRect selected_tile_rect;
    selected_tile_rect.x = float((SCREEN_WIDTH/2) - (this->map.tile_width/2));
    selected_tile_rect.y = float((SCREEN_HEIGHT/2) - (this->map.tile_height/2));
    selected_tile_rect.w = float(this->map.tile_width);
    selected_tile_rect.h = float(this->map.tile_height);

    SDL_SetRenderTarget(renderer, this->texture);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, this->map.bg, &src, &dst);

    // only render the foreground if not only background
    if (this->layer != BACKGROUND)
        SDL_RenderTexture(renderer, this->map.fg, &src, &dst);

    // render collision layer if that mode is selected
    if (this->layer == COLLISION)
        SDL_RenderTexture(renderer, this->collision_texture, &src, &dst);

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
            selected_tile_src_rect.x = float((this->sel_collider+1) * this->map.tile_width);
            selected_tile_src_rect.y = 0.0f;
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
        vertices[i].color.a = 1.0f;
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
