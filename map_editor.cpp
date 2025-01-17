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
    INPUT_ADD_SHEET
};

class Editor : public Object {
public:
    Editor(const char *map_path);
    ~Editor();

    void step() override;
    void render();

private:
    void input_add_sheet();

    std::string map_path;
    Map map;

    bool fg_toggle = false;
    bool user_inputting = false;
    EditorInputMode input_mode = INPUT_ADD_SHEET;
    
    std::string text;
    int sel_x = 0, sel_y = 0, sel_tilesheet = -1, sel_ts_x = 0, sel_ts_y = 0;
    SDL_FRect viewing;
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
    }

    this->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    this->src_rect = new SDL_FRect();
    this->src_rect->x = this->dst_rect.x = 0.0f;
    this->src_rect->y = this->dst_rect.y = 0.0f;
    this->src_rect->w = this->dst_rect.w = float(SCREEN_WIDTH);
    this->src_rect->h = this->dst_rect.h = float(SCREEN_HEIGHT);

    this->map_path = map_path;
    this->map_path += ".edited"; // TEMPORARY

    if (!this->map.tilesheets.empty())
        this->sel_tilesheet = 0;
}

Editor::~Editor() {
    SDL_DestroyTexture(this->texture);
    this->map.write(this->map_path.c_str());
    delete this->src_rect;
}

void Editor::step() {
    if (this->user_inputting) {
        switch (this->input_mode) {
        case INPUT_ADD_SHEET:
            this->input_add_sheet();
            break;
        default:
            this->user_inputting = false;
            break;
        }
    } else {
        // move the selected tile on input
        this->sel_x += int(keyboard.is_hit(SDLK_RIGHT)) - int(keyboard.is_hit(SDLK_LEFT));
        this->sel_y += int(keyboard.is_hit(SDLK_DOWN)) - int(keyboard.is_hit(SDLK_UP));
        this->sel_x = clamp(this->sel_x, 0, this->map.cols - 1);
        this->sel_y = clamp(this->sel_y, 0, this->map.rows - 1);

        std::vector<Tile> *vec = this->map.bg_tiles[(this->sel_y * this->map.cols) + this->sel_x];

        game->draw_text("[T]ile, [S]heet, [A]dd sheet, [F]oreground\n<Return> to push, <Backspace> to pop", 0, 0);
        char status_text[256];
        snprintf(status_text, sizeof(status_text), "%s: %s %d,%d [%d]\n%s %d,%d", this->map_path.c_str(),
            this->fg_toggle ? "fg" : "bg", this->sel_x, this->sel_y, vec == nullptr ? 0 : int(vec->size()),
            this->sel_tilesheet >= 0 ? this->map.tilesheets[this->sel_tilesheet]->path.c_str() : "NULL", this->sel_ts_x, this->sel_ts_y);
        game->draw_text(std::string(status_text), 0, SCREEN_HEIGHT - 22);

        if (keyboard.is_hit(SDLK_RETURN) && this->sel_tilesheet >= 0) {
            // place a tile
            if (vec == nullptr) {
                vec = new std::vector<Tile>();
                this->map.bg_tiles[(this->sel_y * this->map.cols) + this->sel_x] = vec;
            }
            Tile new_tile;
            new_tile.tilesheet = this->sel_tilesheet;
            new_tile.x = this->sel_ts_x;
            new_tile.y = this->sel_ts_y;
            vec->push_back(new_tile);
            this->map.render_tile(this->sel_x, this->sel_y);
        } else if (keyboard.is_hit(SDLK_BACKSPACE)) {
            // remove a tile
            if (vec != nullptr) {
                vec->pop_back();
                if (vec->empty()) {
                    delete vec;
                    vec = nullptr;
                    this->map.bg_tiles[(this->sel_y * this->map.cols) + this->sel_x] = nullptr;
                }
                this->map.render_tile(this->sel_x, this->sel_y);
            }
        } else if (keyboard.is_hit(SDLK_A)) {
            this->user_inputting = true;
            this->input_mode = INPUT_ADD_SHEET;
            this->text = "";
        }
    }

    this->render();
}

void Editor::render() {
    SDL_FRect dst_viewing;

    int viewing_x = ((this->sel_x*this->map.tile_width) + (this->map.tile_width/2)) - (SCREEN_WIDTH/2);
    int viewing_y = ((this->sel_y*this->map.tile_height) + (this->map.tile_height/2)) - (SCREEN_HEIGHT/2);

    if (viewing_x < 0) {
        dst_viewing.x = float(-1 * viewing_x);
        dst_viewing.w = float(min(SCREEN_WIDTH, this->map.tile_width * this->map.cols));
        this->viewing.x = 0.0f;
    } else {
        dst_viewing.x = 0.0f;
        dst_viewing.w = float(min(SCREEN_WIDTH, (this->map.tile_width * this->map.cols) - viewing_x));
        this->viewing.x = float(viewing_x);
    }

    if (viewing_y < 0) {
        dst_viewing.y = float(-1 * viewing_y);
        dst_viewing.h = float(min(SCREEN_HEIGHT, this->map.tile_height * this->map.rows));
        this->viewing.y = 0.0f;
    } else {
        dst_viewing.y = 0.0f;
        dst_viewing.h = float(min(SCREEN_HEIGHT, (this->map.tile_height * this->map.rows) - viewing_y));
        this->viewing.y = float(viewing_y);
    }

    this->viewing.w = dst_viewing.w;
    this->viewing.h = dst_viewing.h;

    SDL_FRect selected_tile_rect;
    selected_tile_rect.x = float((SCREEN_WIDTH/2) - (this->map.tile_width/2));
    selected_tile_rect.y = float((SCREEN_HEIGHT/2) - (this->map.tile_height/2));
    selected_tile_rect.w = float(this->map.tile_width);
    selected_tile_rect.h = float(this->map.tile_height);

    SDL_SetRenderTarget(renderer, this->texture);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, this->map.bg, &this->viewing, &dst_viewing);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
    SDL_RenderRect(renderer, &selected_tile_rect);
    SDL_SetRenderTarget(renderer, game->screen);
}

void Editor::input_add_sheet() {
    if (keyboard.is_hit(SDLK_BACKSPACE) && !text.empty())
        this->text.pop_back();
    else if (keyboard.c != NO_CHAR)
        this->text.push_back(keyboard.c);
    else if (keyboard.is_hit(SDLK_RETURN)) {
        // add new tilesheet
        this->user_inputting = false;
        Tilesheet *tilesheet = new Tilesheet(this->text.c_str(), this->map.tile_width, this->map.tile_height);
        if (tilesheet->texture != nullptr) {
            this->map.tilesheets.push_back(tilesheet);
            this->sel_tilesheet = this->map.tilesheets.size() - 1;
        } else {
            delete tilesheet;
        }
    } else if (keyboard.is_hit(SDLK_ESCAPE)) {
        // cancel
        this->user_inputting = false;
    }

    game->draw_text("Tilesheet path: " + this->text, 0, 0);
    game->draw_text("<Return> to add, <Escape> to cancel", 0, 10);
}
