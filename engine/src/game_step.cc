#include <iostream>

#include "SDL3/SDL_blendmode.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_timer.h"
#include "audio_playback.h"
#include "bmp_texture.h"
#include "controller.h"
#include "font.h"
#include "game.h"
#include "net_agent.h"
#include "save_data.h"

#define FATALITY(errfunc)                                              \
  {                                                                    \
    std::cerr << errfunc << " error: " << SDL_GetError() << std::endl; \
    std::exit(1);                                                      \
  }

#define DELETE_IF_NOT_NULLPTR(thing) \
  if (thing != nullptr) {            \
    delete thing;                    \
    thing = nullptr;                 \
  }

std::unordered_map<uint64_t, std::string> debug_obj_ptrs;

Game::Game() {
  // create textures
  this->screen =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                        SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
  this->ui =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
  this->overlay =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
  if (this->screen == nullptr || this->ui == nullptr ||
      this->overlay == nullptr)
    FATALITY("SDL_CreateTexture")

  SDL_SetTextureScaleMode(this->screen, SDL_SCALEMODE_NEAREST);

  this->draw_rect(this->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
  this->draw_rect(this->overlay, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

  // load the ui box texture
  SDL_Surface* ui_box_surface = SDL_LoadBMP("sprites/ui_box.bmp");
  if (ui_box_surface == nullptr) FATALITY("SDL_LoadBMP")

  this->ui_box = SDL_CreateTextureFromSurface(renderer, ui_box_surface);
  if (this->ui_box == nullptr) FATALITY("SDL_CreateTextureFromSurface")
  SDL_DestroySurface(ui_box_surface);

  SDL_SetTextureScaleMode(this->ui_box, SDL_SCALEMODE_NEAREST);

  // load icons
  SDL_Surface* icons_surface = SDL_LoadBMP("sprites/icons.bmp");
  if (icons_surface == nullptr) FATALITY("SDL_LoadBMP")

  this->icons = SDL_CreateTextureFromSurface(renderer, icons_surface);
  if (this->icons == nullptr) FATALITY("SDL_CreateTextureFromSurface")
  SDL_DestroySurface(icons_surface);

  SDL_SetTextureScaleMode(this->icons, SDL_SCALEMODE_NEAREST);

  this->ticks = SDL_GetTicks();

  Font::load_fonts(this->fonts);

  this->item_count_icon = {HUD_MAIN_X + 18, HUD_MAIN_Y - 2, 16.0f, 16.0f};
  this->item_cooldown_icon = {HUD_MAIN_X + 8, HUD_MAIN_Y + 4, 16.0f, 16.0f};

  load_render_functions();  // bmp_texture.h

  init_playback();  // audio_stream.h
}

Game::~Game() {
  this->unload();

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

  // free factories
  for (auto pair : this->factories) delete pair.second;
  this->factories.clear();

  // free fonts
  for (auto font : this->fonts) delete font;
  this->fonts.clear();

  free_textures();  // bmp_texture.h

  free_playback();  // audio_stream.h
}

void Game::unload() {
  this->save_objects();

  this->deleting_objects = true;

  if (!this->objects.empty()) {
    for (int i = 0; i < this->objects.size(); i++) {
      delete this->objects[i];
      this->objects[i] = nullptr;
    }

    this->objects.clear();
  }

  this->deleting_objects = false;
  this->current_map = "";

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

void Game::load_map(const char* map_path) {
  // if cheddar and connected, notify feta that we are changing maps
  if (this->create_objects &&
      this->net_state == NetworkAgent::State::CONNECTED) {
    net_agent->send_message(MSG_MAP + std::string(map_path) + '\n');
  }

  Map map;
  map.read(map_path);

  this->map_title = map.title;
  this->map_description = map.description;

  this->bg = map.bg;
  this->fg = map.fg;
  this->collision = map.collision;
  this->tile_width = map.tile_width;
  this->tile_height = map.tile_height;
  this->cols = map.cols;
  this->rows = map.rows;
  this->bg_r = map.bg_r;
  this->bg_g = map.bg_g;
  this->bg_b = map.bg_b;

  if (this->ambience != map.ambience) {
    load_audio(map.ambience);
    loop_audio(map.ambience);
  }

  this->ambience = map.ambience;

  // assemble appropriately sized quads for each predefined map collider
  for (int i = 0; i < n_MapColliders; i++) {
    // a quad has four vertices
    for (int j = 0; j < 4; j++) {
      this->colliders[i].vertex[j].x =
          float(map.tile_width) * MapColliders[i][j].x;
      this->colliders[i].vertex[j].y =
          float(map.tile_height) * MapColliders[i][j].y;
    }
  }

  this->current_map = map_path;
  this->creating_objects = true;

  if (this->create_objects || std::string(map_path) == "maps/init") {
    for (auto obj : map.objects) this->create_object(obj.first, obj.second);

    while (!this->new_objects.empty()) {
      auto obj = this->new_objects.front();
      this->new_objects.pop();
      this->create_object(obj.first, obj.second);
    }
  }

  this->creating_objects = false;

  map.clear();

  if (save.geti(LOAD_SAVE) == 1) save.data.erase(LOAD_SAVE);

  save.data[LOAD_MAP] = this->current_map;

  this->display_notification(this->map_title + ", " + this->map_description);

  this->corner_x = 0;
  this->corner_y = 0;
}

void Game::create_object(const std::string& id, const std::string& options) {
  if (this->factories.find(id) == this->factories.end() && id != FIRST_OBJ &&
      id != LAST_OBJ) {
    std::cerr << "Game::create_object error: '" << id << "' does not exist"
              << std::endl;
    return;
  }

  this->delete_object = false;

  Object* obj = this->factories[id]->create(options);
  bool obj_cancelled = false;

  // object cancelled being created
  if (this->delete_object == true) {
    delete obj;
    obj = nullptr;
    this->delete_object = false;
    obj_cancelled = true;
  }

  if (obj != nullptr) {
    if (id == FIRST_OBJ) {
      this->first_obj = obj;
    } else if (id == LAST_OBJ) {
      this->last_obj = obj;
    } else {
      this->objects.push_back(obj);
    }

    debug_obj_ptrs[(uint64_t)obj] = id;
  } else if (!obj_cancelled) {
    std::cerr << "Game::create_object error: '" << id << "' resulted in nullptr"
              << std::endl;
  }
}

void Game::push_object(const std::string& id, const std::string& options) {
  this->new_objects.push({id, options});
}

void Game::save_objects() {
  for (auto obj : this->objects) obj->save_data();
}

void Game::step() {
  // toggle controls menu
  Controller* cur_controller =
      capture_controls ? &captured_controller : &local_controller;
  if (cur_controller->is_hit(Button::MENU)) {
    cur_controller->clear_all();
    this->display_controls_menu = !this->display_controls_menu;
    capture_controls = this->display_controls_menu;
  }

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

  this->audio.clear();
  ambience_step();

  // get ticks and calculate delta
  Uint64 new_ticks = SDL_GetTicks();
  this->delta = float(new_ticks - this->ticks) / 1000.0f;
  this->ticks = new_ticks;

  this->net_state = net_agent->get_state();
  this->draw_overlay();

  if (this->first_obj != nullptr) {
    this->first_obj->step();
  }

  // step all objects
  for (auto it = this->objects.begin(); it != this->objects.end();) {
    this->delete_object = false;
    Object* obj = *it;

    if (obj == nullptr) {
      std::cerr << "Game::step: nullptr found in objects" << std::endl;
      it = this->objects.erase(it);
      continue;
    }

    obj->step();

    if (this->delete_object) {
      it = this->objects.erase(it);
      delete obj;
      this->delete_object = false;
      continue;
    } else
      it++;
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
  SDL_SetRenderDrawColor(renderer, this->bg_r, this->bg_g, this->bg_b, 255);
  SDL_RenderClear(renderer);

  // render background
  SDL_FRect map_src = SDL_FRect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
  SDL_FRect map_dst = SDL_FRect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

  if (this->bg != nullptr) {
    this->make_map_rect(this->corner_x, this->corner_y, this->bg->w,
                        this->bg->h, &map_src, &map_dst);
    SDL_RenderTexture(renderer, this->bg, &map_src, &map_dst);
  }

  // make sure all sprites are blended, not replacing, pixels
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  // render objects to the screen
  for (SpriteRender& sprite : this->sprites) {
    // note: returns false if either is NULL, e.g., sprite.dst_rect
    if (SDL_HasRectIntersectionFloat(&map_src, sprite.dst_rect)) {
      SDL_FRect dst_rect = *sprite.dst_rect;
      dst_rect.x -= game->corner_x;
      dst_rect.y -= game->corner_y;
      SDL_RenderTexture(renderer, sprite.texture, sprite.src_rect, &dst_rect);
    }
  }

  this->sprites.clear();  // clear sprites; next frame will repopulate

  // render foreground
  if (this->fg != nullptr)
    SDL_RenderTexture(renderer, this->fg, &map_src, &map_dst);

  // render ui
  SDL_RenderTexture(renderer, this->ui, NULL, NULL);

  // render overlay
  SDL_RenderTexture(renderer, this->overlay, NULL, NULL);

  if (local_controller.c == 'p' || captured_controller.c == 'p') {
    local_controller.c = NO_CHAR;
    SDL_Surface* _screen = SDL_RenderReadPixels(renderer, NULL);
    SDL_SaveBMP(_screen, "screenshot.bmp");
    SDL_DestroySurface(_screen);
  }

  SDL_SetRenderTarget(renderer, NULL);
}
