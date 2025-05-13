#include "SDL3/SDL_blendmode.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_timer.h"
#include "bmp_texture.h"
#include "controller.h"
#include "font.h"
#include "game.h"
#include "net_agent.h"

#include <iostream>

#define FATALITY(errfunc) {\
    std::cerr << errfunc << " error: " << SDL_GetError() << std::endl;\
    std::exit(1);\
}

#define DELETE_IF_NOT_NULLPTR(thing) if (thing != nullptr) {\
    delete thing;\
    thing = nullptr;\
}

Game::Game() {
    // create textures
    this->screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    this->ui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    this->overlay = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (this->screen == nullptr || this->ui == nullptr || this->overlay == nullptr) FATALITY("SDL_CreateTexture")

    SDL_SetTextureScaleMode(this->screen, SDL_SCALEMODE_NEAREST);

    this->draw_rect(this->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
    this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

    // load the ui box texture
    SDL_Surface *ui_box_surface = SDL_LoadBMP("sprites/ui_box.bmp");
    if (ui_box_surface == nullptr) FATALITY("SDL_LoadBMP")

    this->ui_box = SDL_CreateTextureFromSurface(renderer, ui_box_surface);
    if (this->ui_box == nullptr) FATALITY("SDL_CreateTextureFromSurface")

    SDL_SetTextureScaleMode(this->ui_box, SDL_SCALEMODE_NEAREST);

    // load icons
    SDL_Surface *icons_surface = SDL_LoadBMP("sprites/icons.bmp");
    if (icons_surface == nullptr) FATALITY("SDL_LoadBMP")

    this->icons = SDL_CreateTextureFromSurface(renderer, icons_surface);
    if (this->icons == nullptr) FATALITY("SDL_CreateTextureFromSurface")

    SDL_SetTextureScaleMode(this->screen, SDL_SCALEMODE_NEAREST);

    this->ticks = SDL_GetTicks();
    // only show load-in screen upon loading a map
    this->load_ticks = this->ticks;

    Font::load_fonts(this->fonts);
    load_render_functions(); // bmp_texture.h
}

Game::~Game() {
    this->unload();
    free_textures();

    // destroy the screen
    SDL_DestroyTexture(this->screen);
    this->screen = nullptr;

    // destroy the ui texture
    SDL_DestroyTexture(this->ui);
    this->ui = nullptr;

    // destroy the overlay texture
    SDL_DestroyTexture(this->overlay);
    this->overlay = nullptr;

    // destroy the ui box texture
    SDL_DestroyTexture(this->ui_box);
    this->ui_box = nullptr;

    // destroy the icons texture
    SDL_DestroyTexture(this->icons);
    this->icons = nullptr;

    DELETE_IF_NOT_NULLPTR(this->first_obj);
    DELETE_IF_NOT_NULLPTR(this->last_obj);
    DELETE_IF_NOT_NULLPTR(net_agent);

    // free fonts
    for (auto font : this->fonts)
        delete font;
    this->fonts.clear();
}

void Game::unload() {
    this->current_map = "";

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
    this->draw_rect(this->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
}

void Game::load_map(const char *map_path) {
    Map map;
    map.read(map_path);

    this->current_map = map_path;
    this->map_title = map.title;
    this->map_description = map.description;

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

    if (this->create_objects) {
        for (auto obj : map.objects)
            this->create_object(obj.first, obj.second);

        while (!this->new_objects.empty()) {
            auto obj = this->new_objects.front();
            this->new_objects.pop();
            this->create_object(obj.first, obj.second);
        }
    }

    map.clear();

    // map loaded screen
    this->load_ticks = SDL_GetTicks();
}

void Game::create_object(const std::string &id, const std::string &options) {
    if (!this->factories.contains(id) && id != FIRST_OBJ && id != LAST_OBJ) {
        std::cerr << "Game::create_object error: '" << id << "' does not exist" << std::endl;
        return;
    }

    Object *obj = this->factories[id]->create(options);
    if (obj != nullptr) {
        if (id == FIRST_OBJ) {
            this->first_obj = obj;
        } else if (id == LAST_OBJ) {
            this->last_obj = obj;
        } else {
            this->objects.push_back(obj);
        }
    }
}

void Game::push_object(const std::string &id, const std::string &options) {
    this->new_objects.push({ id, options });
}

void Game::save_objects() {
    for (auto obj : this->objects)
        obj->save_data();
}

void Game::post_save_objects() {
    for (auto obj : this->objects)
        obj->post_save_data();
}

void Game::step() {
    // create new map
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

    // get ticks and calculate delta
    Uint64 new_ticks = SDL_GetTicks();
    this->delta = float(new_ticks - this->ticks) / 1000.0f;
    this->ticks = new_ticks;

    this->draw_overlay();

    if (this->first_obj != nullptr) {
        this->first_obj->step();
    }

    // step all objects
    for (auto it = this->objects.begin(); it != this->objects.end();) {
        Object *obj = *it;
        obj->step();

        if (this->delete_object) {
            delete obj;
            it = this->objects.erase(it);
            this->delete_object = false;
        } else it++;
    }

    // create new objects
    while (!this->new_objects.empty()) {
        auto args = this->new_objects.front();
        this->new_objects.pop();
        this->create_object(args.first, args.second);
    }

    if (this->last_obj != nullptr) {
        this->last_obj->step();
    }

    SDL_SetRenderTarget(renderer, this->screen);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // render background
    SDL_FRect map_src, map_dst;
    if (this->bg != nullptr) {
        this->make_map_rect(this->corner_x, this->corner_y, this->bg->w, this->bg->h, &map_src, &map_dst);
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

    // render overlay
    SDL_RenderTexture(renderer, this->overlay, NULL, NULL);

    if (local_controller.c == 'p') {
        local_controller.c = NO_CHAR;
        SDL_Surface *_screen = SDL_RenderReadPixels(renderer, NULL);
        SDL_SaveBMP(_screen, "screenshot.bmp");
        SDL_DestroySurface(_screen);
    }

    SDL_SetRenderTarget(renderer, NULL);
}
