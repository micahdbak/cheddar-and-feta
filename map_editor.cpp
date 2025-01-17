#include <iostream>
#include <vector>

#include "game.h"
#include "keyboard.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

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

    std::string map_path;
    Map map;

    bool fg_toggle = false;
    bool user_inputting = false;
    EditorInputMode input_mode;
    
    std::string text;
    int sel_x = 0, sel_y = 0, sel_tilesheet = -1, sel_ts_x = 0, sel_ts_y = 0;
};

class EditorFactory : public ObjectFactory {
public:
    EditorFactory() {}
    ~EditorFactory() {}

    Object *create(std::string options) override {
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
    this->src_rect = new SDL_FRect();
    this->src_rect->x = this->dst_rect.x = 0.0f;
    this->src_rect->y = this->dst_rect.y = 0.0f;
    this->src_rect->w = this->dst_rect.w = float(SCREEN_WIDTH);
    this->src_rect->h = this->dst_rect.h = float(SCREEN_HEIGHT);

    this->map_path = map_path;

    if (!this->map.tilesheets.empty())
        this->sel_tilesheet = 0;
}

Editor::~Editor() {
    SDL_DestroyTexture(this->texture);
    delete this->src_rect;
    if (this->map.bg != nullptr) {
        SDL_DestroyTexture(this->map.bg);
        this->map.bg = nullptr;
    }
    if (this->map.fg != nullptr) {
        SDL_DestroyTexture(this->map.fg);
        this->map.fg = nullptr;
    }
}

void Editor::step() {
    if (!this->user_inputting) {
        // move the selected tile on input
        this->sel_x += int(keyboard.is_hit(SDLK_RIGHT)) - int(keyboard.is_hit(SDLK_LEFT));
        this->sel_y += int(keyboard.is_hit(SDLK_DOWN)) - int(keyboard.is_hit(SDLK_UP));
        this->sel_x = clamp(this->sel_x, 0, this->map.cols - 1);
        this->sel_y = clamp(this->sel_y, 0, this->map.rows - 1);

        std::vector<Tile> *vec;
        if (this->fg_toggle) {
            vec = this->map.fg_tiles[(this->sel_y * this->map.cols) + this->sel_x];
        } else {
            vec = this->map.bg_tiles[(this->sel_y * this->map.cols) + this->sel_x];
        }

        game->draw_text(" [T]ile, [S]heet, [A]dd, [F]g/bg, [W]rite, [O]bjects  ", 0, 0);
        char status_text[256];
        snprintf(status_text, sizeof(status_text), "%s: %s %d,%d [%d]", this->map_path.c_str(),
            this->fg_toggle ? "fg" : "bg", this->sel_x * this->map.tile_width, this->sel_y * this->map.tile_height, vec == nullptr ? 0 : int(vec->size()));
        game->draw_text(std::string(status_text), 0, SCREEN_HEIGHT - 11);

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
        case 'f':
            this->fg_toggle = !this->fg_toggle;
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
            bool should_place = true;

            // place a tile
            if (vec == nullptr) {
                vec = new std::vector<Tile>();
                if (this->fg_toggle) {
                    this->map.fg_tiles[(this->sel_y * this->map.cols) + this->sel_x] = vec;
                } else {
                    this->map.bg_tiles[(this->sel_y * this->map.cols) + this->sel_x] = vec;
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
        } else if (keyboard.is_hit(SDLK_BACKSPACE) || keyboard.is_down(SDLK_MINUS)) {
            // remove a tile
            if (vec != nullptr) {
                vec->pop_back();
                if (vec->empty()) {
                    delete vec;
                    vec = nullptr;
                    if (this->fg_toggle) {
                        this->map.fg_tiles[(this->sel_y * this->map.cols) + this->sel_x] = nullptr;
                    } else {
                        this->map.bg_tiles[(this->sel_y * this->map.cols) + this->sel_x] = nullptr;
                    }
                }
                this->map.render_tile(this->sel_x, this->sel_y);
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
            break;
        }
    }
}

void Editor::input_tile() {
    this->sel_ts_x += int(keyboard.is_hit(SDLK_RIGHT)) - int(keyboard.is_hit(SDLK_LEFT));
    this->sel_ts_y += int(keyboard.is_hit(SDLK_DOWN)) - int(keyboard.is_hit(SDLK_UP));
    this->sel_ts_x = clamp(this->sel_ts_x, 0, this->map.tilesheets[this->sel_tilesheet]->cols - 1);
    this->sel_ts_y = clamp(this->sel_ts_y, 0, this->map.tilesheets[this->sel_tilesheet]->rows - 1);

    if (keyboard.is_hit(SDLK_RETURN) || keyboard.is_hit(SDLK_ESCAPE)) {
        this->user_inputting = false;
    }

    SDL_SetRenderTarget(renderer, this->texture);
    SDL_FRect tilesheet_rect;
    tilesheet_rect.x = 0.0f;
    tilesheet_rect.y = 11.0f;
    tilesheet_rect.w = this->map.tilesheets[this->sel_tilesheet]->texture->w;
    tilesheet_rect.h = this->map.tilesheets[this->sel_tilesheet]->texture->h;
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &tilesheet_rect);
    SDL_RenderTexture(renderer, this->map.tilesheets[this->sel_tilesheet]->texture, NULL, &tilesheet_rect);
    SDL_FRect sel_tile_rect;
    sel_tile_rect.w = this->map.tile_width;
    sel_tile_rect.h = this->map.tile_height;
    sel_tile_rect.x = this->map.tile_width * this->sel_ts_x;
    sel_tile_rect.y = tilesheet_rect.y + this->map.tile_height * this->sel_ts_y;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 240);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderRect(renderer, &sel_tile_rect);
    SDL_SetRenderTarget(renderer, game->screen);

    game->draw_text("<Return> to place, <Escape> to close", 0, 0);
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
    game->draw_text(display_text, 0, 0);
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
    }

    game->draw_text("Tilesheet path: " + this->text + "\n<Return> to add, <Escape> to close", 0, 0);
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
    game->draw_text(display_text, 0, 0);
}

void Editor::render() {
    SDL_FRect src, dst;

    int x = (this->sel_x*this->map.tile_width) + (this->map.tile_width/2);
    int y = (this->sel_y*this->map.tile_height) + (this->map.tile_height/2);

    game->make_map_rect(x, y, this->map.tile_width * this->map.cols, this->map.tile_height * this->map.rows, &src, &dst);

    SDL_FRect selected_tile_rect;
    selected_tile_rect.x = float((SCREEN_WIDTH/2) - (this->map.tile_width/2));
    selected_tile_rect.y = float((SCREEN_HEIGHT/2) - (this->map.tile_height/2));
    selected_tile_rect.w = float(this->map.tile_width);
    selected_tile_rect.h = float(this->map.tile_height);

    SDL_SetRenderTarget(renderer, this->texture);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, this->map.bg, &src, &dst);

    // only render the foreground if it is toggled on
    if (this->fg_toggle) {
        SDL_RenderTexture(renderer, this->map.fg, &src, &dst);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 240);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderRect(renderer, &selected_tile_rect);

    if (this->sel_tilesheet >= 0) {
        selected_tile_rect.x = 0.0f;
        selected_tile_rect.y = float(SCREEN_HEIGHT - 11 - this->map.tile_height);
        SDL_FRect selected_tile_src_rect;
        selected_tile_src_rect.x = float(this->sel_ts_x * this->map.tile_width);
        selected_tile_src_rect.y = float(this->sel_ts_y * this->map.tile_height);
        selected_tile_src_rect.w = float(this->map.tile_width);
        selected_tile_src_rect.h = float(this->map.tile_height);
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderFillRect(renderer, &selected_tile_rect);
        SDL_RenderTexture(renderer, this->map.tilesheets[this->sel_tilesheet]->texture, &selected_tile_src_rect, &selected_tile_rect);
    }

    SDL_SetRenderTarget(renderer, game->screen);
}
