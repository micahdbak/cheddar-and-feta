#include "editor.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "colour.h"
#include "editor_ui.h"
#include "hud/hud.h"
#include "input/controller.h"
#include "input/input.h"
#include "map/map.h"
#include "map/map_collider.h"
#include "map/map_layer.h"
#include "map/tilesheet.h"
#include "object.h"
#include "renderer/font.h"
#include "renderer/renderer.h"
#include "scene/scene.h"
#include "texture/texture.h"
#include "thoom/macros.h"
#include "window/window.h"

static void parse_billboard_options(const std::string& options, int* x, int* y,
                                    int* ts_x, int* ts_y, int* w, int* h,
                                    int* depth, std::string& tilesheet) {
  char buff[1024];
  if (sscanf(options.c_str(), "%d,%d,%d,%d,%d,%d,%d,%1023[^\n]", x, y, ts_x,
             ts_y, w, h, depth, buff) < 8) {
    std::cerr << "parse_billboard_options error: bad options: " << options
              << std::endl;
    std::exit(1);
  }

  tilesheet = buff;
}

Editor::Editor(const std::vector<std::string>& args) {
  this->map_path = args[0];

  std::string bin_path = args[0] + ".bin";
  FILE* file = fopen(bin_path.c_str(), "rb");
  if (file == nullptr) {
    if (args.size() < 7) {
      std::cerr << "To create a new map please provide: <map name> <tw> <th> "
                   "<cols> <rows> <fill(0/1)> <fillts(opt)>."
                << std::endl;
      std::exit(1);
    }

    int tile_width = atoi(args[1].c_str());
    int tile_height = atoi(args[2].c_str());
    int cols = atoi(args[3].c_str());
    int rows = atoi(args[4].c_str());
    bool should_fill = atoi(args[5].c_str());

    this->map = new thoom::Map(tile_width, tile_height, cols, rows);
    this->map->path = this->map_path;

    if (should_fill) {
      if (!this->map->add_tilesheet(args[6].c_str())) {
        std::cerr << args[6] << " is not a valid tilesheet." << std::endl;
        std::exit(1);
      }

      thoom::Tilesheet* tilesheet = this->map->tilesheets[0];
      int fill_cols = THOOM_MIN(4, tilesheet->cols);
      std::vector<thoom::Tile> tiles;

      for (int x = 0; x < fill_cols; x++) {
        tiles.push_back(thoom::Tile{0, static_cast<unsigned int>(x), 0});
      }

      this->map->layers[thoom::Map::kBackgroundLayer]->fill_randomly(tiles);

      for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
          this->map->render_tile(x, y);
        }
      }
    }
  } else {
    fclose(file);
    this->map = new thoom::Map(this->map_path.c_str());
    std::string backup_path = this->map_path;
    backup_path += ".backup";
    this->map->write(backup_path.c_str());
  }

  this->ui.init(this->map);

  this->view_texture = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_LINEAR);
  this->viewport = SDL_FRect{0.0f, 0.0f, float(THOOM_SCREEN_WIDTH),
                             float(THOOM_SCREEN_HEIGHT)};
  this->world_rect = this->viewport;

  this->objects_texture = thoom::Texture::instance->create_texture(
      this->map->layers[thoom::Map::kBackgroundLayer]->texture->w,
      this->map->layers[thoom::Map::kBackgroundLayer]->texture->h,
      SDL_PIXELFORMAT_RGBA8888, SDL_SCALEMODE_LINEAR);
  SDL_SetTextureAlphaMod(this->objects_texture, 0xa0);
}

Editor::~Editor() {
  if (this->view_texture != nullptr) {
    SDL_DestroyTexture(this->view_texture);
    this->view_texture = nullptr;
  }

  if (this->objects_texture != nullptr) {
    SDL_DestroyTexture(this->objects_texture);
    this->objects_texture = nullptr;
  }

  delete this->map;
  this->map = nullptr;
}

void Editor::step() {
  thoom::Input* input = thoom::Input::instance;

  if (!this->ui.inputting) {
    this->ui.move(this->map);
    this->draw_status();

    switch (input->world.character) {
      case 'h':
        this->map->write(this->map_path.c_str());
        break;
      case 'j':
        if (this->layer == COLLISION || this->ui.tilesheet >= 0) {
          this->ui.begin_tile();
        }
        break;
      case 'k':
        if (!this->map->tilesheets.empty()) {
          this->ui.begin_sheet();
        }
        break;
      case 'l':
        this->cycle_layer();
        break;
      case 'x': {
        std::cout << "exporting map to mapdump.bmp" << std::endl;
        SDL_Surface* background = thoom::Window::instance->read_pixels(
            this->map->layers[thoom::Map::kBackgroundLayer]->texture);
        SDL_SaveBMP(background, "mapdump.bmp");
        SDL_DestroySurface(background);
      } break;
      default:
        break;
    }
    input->world.character = '\0';

    if (!this->ui.inputting) {
      this->handle_placement();
    }
  }

  if (this->objects_dirty) {
    this->render_objects();
    this->objects_dirty = false;
  }

  int center_x =
      (this->ui.tile_x * this->map->tile_width) + (this->map->tile_width / 2);
  int center_y =
      (this->ui.tile_y * this->map->tile_height) + (this->map->tile_height / 2);
  this->set_camera(center_x, center_y);

  this->draw_view();
  this->ui.render(this->view_texture, this->map, this->layer);

  if (this->ui.inputting) {
    this->ui.step(this->map, this->layer);

    if (this->ui.inputting) {
      this->ui.render_input(this->view_texture, this->map, this->layer);
    } else {
      thoom::Renderer::instance->clear(thoom::Hud::instance->ui, thoom::kMask);
    }
  }

  thoom::Renderer::instance->push_sprite("EDITOR", this->view_texture,
                                         &this->viewport, &this->world_rect, 0);
}

thoom::Object* Editor::create_object(thoom::Scene* scene,
                                     const std::string& options) {
  std::vector<std::string> args;
  std::string arg;

  for (char c : options) {
    if (c == '\n') {
      args.push_back(arg);
      arg.clear();
    } else {
      arg += c;
    }
  }

  args.push_back(arg);
  return new Editor(args);
}

thoom::MapLayer* Editor::map_layer() const {
  switch (this->layer) {
    case BACKGROUND:
      return this->map->layers[thoom::Map::kBackgroundLayer];
    case FOREGROUND:
      return this->map->layers[thoom::Map::kForegroundLayer];
    default:
      return nullptr;
  }
}

void Editor::draw_status() const {
  thoom::MapLayer* map_layer = this->map_layer();
  int count = 0;

  if (map_layer != nullptr) {
    count = map_layer->tile_count(this->ui.tile_x, this->ui.tile_y);
  }

  SDL_FRect black_rect = {0.0f, 0.0f, float(THOOM_SCREEN_WIDTH), 11.0f};
  thoom::Renderer::instance->draw_rect(thoom::Hud::instance->ui, &black_rect,
                                       thoom::Colour(0, 0, 0),
                                       SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_text(
      thoom::Hud::instance->ui, MONO_FONT,
      " h:write, j:tile, k:sheet, l:layer, x:export  ", 0, 0, 0,
      kBackground);

  char status_text[256];

  if (this->layer != COLLISION) {
    snprintf(status_text, sizeof(status_text), "%s: %s %d,%d [%d]",
             this->map_path.c_str(), this->layer == BACKGROUND ? "bg" : "fg",
             this->ui.tile_x * this->map->tile_width,
             this->ui.tile_y * this->map->tile_height, count);
  } else {
    snprintf(status_text, sizeof(status_text), "%s: %s %d,%d",
             this->map_path.c_str(), "collision",
             this->ui.tile_x * this->map->tile_width,
             this->ui.tile_y * this->map->tile_height);
  }

  black_rect = {0.0f, float(THOOM_SCREEN_HEIGHT - 11),
                float(THOOM_SCREEN_WIDTH), 11.0f};
  thoom::Renderer::instance->draw_rect(thoom::Hud::instance->ui, &black_rect,
                                       thoom::Colour(0, 0, 0),
                                       SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_text(
      thoom::Hud::instance->ui, MONO_FONT, std::string(status_text), 0,
      THOOM_SCREEN_HEIGHT - 11, 0, kBackground);
}

void Editor::handle_placement() {
  thoom::Input* input = thoom::Input::instance;

  if (input->world.consume_hit(thoom::Button::ATTACK) &&
      this->ui.tilesheet >= 0 && this->ui.region_x >= 0 &&
      this->layer != COLLISION) {
    int left = THOOM_MIN(this->ui.sheet_x, this->ui.region_x);
    int top = THOOM_MIN(this->ui.sheet_y, this->ui.region_y);
    int width = THOOM_ABS(this->ui.sheet_x - this->ui.region_x) + 1;
    int height = THOOM_ABS(this->ui.sheet_y - this->ui.region_y) + 1;

    this->map->add_region(this->layer, this->ui.tile_x, this->ui.tile_y,
                          this->ui.tilesheet, left, top, width, height);
  } else if (input->world.consume_hit(thoom::Button::TOSS) &&
             this->ui.tilesheet >= 0 && this->ui.region_x >= 0 &&
             this->layer != COLLISION) {
    int ts_x = THOOM_MIN(this->ui.sheet_x, this->ui.region_x);
    int ts_y = THOOM_MIN(this->ui.sheet_y, this->ui.region_y);
    int width = (THOOM_ABS(this->ui.sheet_x - this->ui.region_x) + 1) *
                this->map->tile_width;
    int height = (THOOM_ABS(this->ui.sheet_y - this->ui.region_y) + 1) *
                 this->map->tile_height;

    char buff[1024];
    snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d,%d,%d,%s",
             this->ui.tile_x * this->map->tile_width,
             this->ui.tile_y * this->map->tile_height, ts_x, ts_y, width,
             height, height,
             this->map->tilesheets[this->ui.tilesheet]->path.c_str());

    thoom::ObjectRequest object;
    object.id = "billboard";
    object.options = buff;
    this->map->add_object(object);

    this->objects_dirty = true;
  } else if (input->world.down[thoom::Button::SELECT] &&
             (this->layer == COLLISION || this->ui.tilesheet >= 0)) {
    if (this->layer == COLLISION) {
      this->map->collider->set(this->ui.tile_x, this->ui.tile_y,
                               this->ui.collider);
    } else {
      thoom::MapLayer* map_layer = this->map_layer();
      thoom::Tile tile;
      tile.tilesheet = static_cast<unsigned int>(this->ui.tilesheet);
      tile.x = static_cast<unsigned int>(this->ui.sheet_x);
      tile.y = static_cast<unsigned int>(this->ui.sheet_y);

      if (map_layer != nullptr &&
          map_layer->place_tile(this->ui.tile_x, this->ui.tile_y, tile)) {
        this->map->render_tile(this->ui.tile_x, this->ui.tile_y);
      }
    }
  } else if (input->world.consume_hit(thoom::Button::CANCEL)) {
    if (this->layer == COLLISION) {
      this->map->collider->set(this->ui.tile_x, this->ui.tile_y, -1);
    } else {
      thoom::MapLayer* map_layer = this->map_layer();

      if (map_layer != nullptr &&
          map_layer->remove_tile(this->ui.tile_x, this->ui.tile_y)) {
        this->map->render_tile(this->ui.tile_x, this->ui.tile_y);
      }
    }
  }
}

void Editor::cycle_layer() {
  switch (this->layer) {
    case BACKGROUND:
      this->layer = FOREGROUND;
      break;
    case FOREGROUND:
      this->layer = COLLISION;
      break;
    default:
      this->layer = BACKGROUND;
      break;
  }
}

void Editor::set_camera(int x, int y) {
  int corner_x = x - (THOOM_SCREEN_WIDTH / 2);
  int corner_y = y - (THOOM_SCREEN_HEIGHT / 2);

  this->world_rect =
      SDL_FRect{float(corner_x), float(corner_y), float(THOOM_SCREEN_WIDTH),
                float(THOOM_SCREEN_HEIGHT)};

  thoom::Renderer::instance->set_view(float(x), float(y));
}

void Editor::draw_view() {
  SDL_FRect src_rect;
  SDL_FRect dst_rect;
  thoom::Renderer::instance->map_rect(this->map, &this->viewport, &src_rect,
                                      &dst_rect);

  thoom::Renderer::instance->clear(this->view_texture,
                                   thoom::Colour(128, 128, 128));
  thoom::Renderer::instance->draw_map_layer(this->view_texture, this->map,
                                            &this->viewport,
                                            thoom::Map::kBackgroundLayer, 1.0f);

  float foreground_alpha = this->layer == BACKGROUND ? 0.5f : 1.0f;
  thoom::Renderer::instance->draw_map_layer(
      this->view_texture, this->map, &this->viewport,
      thoom::Map::kForegroundLayer, foreground_alpha);

  if (this->layer == COLLISION) {
    thoom::Renderer::instance->draw_texture(
        this->view_texture, this->objects_texture, &src_rect, &dst_rect);
    thoom::Renderer::instance->draw_collision(
        this->view_texture, this->map, &this->viewport,
        thoom::Colour(255, 255, 255, 128));
  }
}

void Editor::render_objects() {
  thoom::Renderer::instance->clear(this->objects_texture, thoom::kMask);

  for (const thoom::ObjectRequest& object : this->map->objects) {
    if (object.id == "billboard") {
      this->draw_billboard(object);
    } else {
      this->draw_object(object);
    }
  }
}

void Editor::draw_billboard(const thoom::ObjectRequest& object) {
  int x, y, ts_x, ts_y, w, h, depth;
  std::string tilesheet;

  parse_billboard_options(object.options, &x, &y, &ts_x, &ts_y, &w, &h, &depth,
                          tilesheet);

  SDL_Texture* tex = thoom::Texture::instance->load(tilesheet);

  SDL_FRect src_rect;
  src_rect.x = float(ts_x * this->map->tile_width);
  src_rect.y = float(ts_y * this->map->tile_height);
  src_rect.w = float(w);
  src_rect.h = float(h);
  SDL_FRect dst_rect = SDL_FRect{float(x), float(y), float(w), float(h)};

  SDL_SetTextureAlphaMod(tex, 0x40);
  thoom::Renderer::instance->draw_texture(this->objects_texture, tex, &src_rect,
                                          &dst_rect);
  SDL_SetTextureAlphaMod(tex, 0xff);

  thoom::Renderer::instance->draw_outline(this->objects_texture, &dst_rect,
                                          thoom::Colour(255, 128, 0),
                                          SDL_BLENDMODE_NONE);
}

void Editor::draw_object(const thoom::ObjectRequest& object) {
  int x, y;
  if (sscanf(object.options.c_str(), "%d,%d", &x, &y) == 2) {
    SDL_FRect dst_rect =
        SDL_FRect{float(x) - 8.0f, float(y) - 8.0f, 16.0f, 16.0f};
    thoom::Renderer::instance->draw_outline(this->objects_texture, &dst_rect,
                                            thoom::kOrange, SDL_BLENDMODE_NONE);
    thoom::Renderer::instance->draw_text(this->objects_texture, SMALL_FONT,
                                         object.id, x + 6, y - 4, 0,
                                         kBackground);
  }
}
