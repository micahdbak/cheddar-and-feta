#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "collider.h"
#include "colour.h"
#include "constants.h"
#include "controller.h"
#include "font.h"
#include "hud.h"
#include "icon.h"
#include "input.h"
#include "macros.h"
#include "map.h"
#include "map_layer.h"
#include "object.h"
#include "peer.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"
#include "tilesheet.h"
#include "window.h"

enum EditorSelectedLayer { BACKGROUND, FOREGROUND, COLLISION };

enum EditorInputMode { INPUT_TILE, INPUT_SHEET };

class Editor : public thoom::Object {
 public:
  Editor(const std::vector<std::string>& args);
  ~Editor();

  void step() override;

  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  void input_tile();
  void input_sheet();
  void render();
  void render_collision_tile(SDL_Texture* target, int x, int y, int collider);
  void render_objects();

  std::string map_path;
  thoom::Map* map = nullptr;

  EditorSelectedLayer layer = BACKGROUND;
  int sel_x = 0, sel_y = 0;
  int sel_tilesheet = -1, sel_ts_x = 0, sel_ts_y = 0, sel_ts2_x = -1,
      sel_ts2_y = -1;
  int sel_collider = -1;

  bool user_inputting = false;
  EditorInputMode input_mode;
  std::string text;

  SDL_Texture* texture = nullptr;
  SDL_FRect src_rect, dst_rect;
  SDL_Texture* objects = nullptr;

  SDL_Texture *collision_texture = nullptr, *collision_sheet = nullptr;
};

void game_init(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Please provide a map to edit via the command line."
              << std::endl;
    std::exit(1);
  }

  std::string options;
  for (int i = 1; i < argc; i++) {
    if (i > 1) options += '\n';
    options += argv[i];
  }

  thoom::Scene::ObjectDef editor;
  editor.id = "editor";
  editor.create_object = Editor::create_object;
  thoom::Scene::instance->register_object(editor);
  thoom::Scene::instance->create_object("editor", options);

  std::string title = "Map Editor - " + std::string(argv[1]);
  thoom::Window::instance->set_title(title.c_str());

  thoom::Peer::init(false);
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

    if (should_fill) {
      thoom::Tilesheet* tilesheet =
          new thoom::Tilesheet(args[6].c_str(), tile_width, tile_height);
      if (tilesheet->texture == nullptr) {
        std::cerr << args[5] << " is not a valid tilesheet." << std::endl;
        std::exit(1);
      }

      this->map->tilesheets.push_back(tilesheet);

      for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
          std::vector<thoom::Tile>* tiles = new std::vector<thoom::Tile>();
          unsigned int tilex = rand() % 4;
          tiles->push_back(thoom::Tile{0, tilex, 0});
          this->map->layers[thoom::Map::kBackgroundLayer]
              ->tiles[(y * cols) + x] = tiles;
          this->map->render_tile(x, y);
        }
      }
    }
  } else {
    fclose(file);
    this->map = new thoom::Map(args[0].c_str());
    std::string backup_path = args[0];
    backup_path += ".backup";
    this->map->write(backup_path.c_str());
  }

  this->texture = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_LINEAR);
  this->src_rect = this->dst_rect =
      SDL_FRect{0, 0, THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT};

  if (!this->map->tilesheets.empty()) this->sel_tilesheet = 0;

  this->collision_texture = thoom::Texture::instance->create_texture(
      this->map->layers[thoom::Map::kBackgroundLayer]->texture->w,
      this->map->layers[thoom::Map::kBackgroundLayer]->texture->h,
      SDL_PIXELFORMAT_RGBA8888, SDL_SCALEMODE_LINEAR);

  int collision_sheet_w = this->map->tile_width * 14;
  int collision_sheet_h = this->map->tile_height * 3;
  this->collision_sheet = thoom::Texture::instance->create_texture(
      collision_sheet_w, collision_sheet_h, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_LINEAR);

  // assemble appropriately sized quads for each collider
  for (int i = 0; i < thoom::n_MapColliders; i++) {
    // a quad has four vertices
    for (int j = 0; j < 4; j++) {
      this->map->collider->quads[i].vertex[j].x =
          float(this->map->tile_width) * thoom::MapColliders[i][j].x;
      this->map->collider->quads[i].vertex[j].y =
          float(this->map->tile_height) * thoom::MapColliders[i][j].y;
    }
  }

  // render the collision texture
  for (int i = 0; i < this->map->cols * this->map->rows; i++) {
    int collider = this->map->collider->collision[i];
    this->render_collision_tile(this->collision_texture, i % this->map->cols,
                                i / this->map->cols, collider);
  }

  // render the collision sheet for the ease of the user
  // start at -1 for no collider, 0.. for colliders
  for (int i = -1; i < int(thoom::n_MapColliders); i++) {
    int x = (i + 1) % 14;
    int y = (i + 1) / 14;
    this->render_collision_tile(this->collision_sheet, x, y, i);
  }

  this->objects = thoom::Texture::instance->create_texture(
      this->map->layers[thoom::Map::kBackgroundLayer]->texture->w,
      this->map->layers[thoom::Map::kBackgroundLayer]->texture->h,
      SDL_PIXELFORMAT_RGBA8888, SDL_SCALEMODE_LINEAR);
  SDL_SetTextureAlphaMod(this->objects, 0xa0);
  this->render_objects();

  this->sel_x = this->map->cols / 2;
  this->sel_y = this->map->rows / 2;
}

Editor::~Editor() {
  SDL_DestroyTexture(this->texture);
  if (this->collision_texture != nullptr) {
    SDL_DestroyTexture(this->collision_texture);
    this->collision_texture = nullptr;
  }
  if (this->collision_sheet != nullptr) {
    SDL_DestroyTexture(this->collision_sheet);
    this->collision_sheet = nullptr;
  }

  delete this->map;
  this->map = nullptr;
}

void Editor::step() {
  if (!this->user_inputting) {
    // move the selected tile on input
    this->sel_x += thoom::Input::instance->world.consume_hit_dx();
    this->sel_y += thoom::Input::instance->world.consume_hit_dy();
    this->sel_x = THOOM_CLAMP(this->sel_x, 0, this->map->cols - 1);
    this->sel_y = THOOM_CLAMP(this->sel_y, 0, this->map->rows - 1);

    std::vector<thoom::Tile>* vec = nullptr;
    int coord = (this->sel_y * this->map->cols) + this->sel_x;
    switch (this->layer) {
      case BACKGROUND:
        vec = this->map->layers[thoom::Map::kBackgroundLayer]->tiles[coord];
        break;
      case FOREGROUND:
        vec = this->map->layers[thoom::Map::kForegroundLayer]->tiles[coord];
        break;
      default: /* pass */
        break;
    }

    SDL_FRect black_rect = {0.0f, 0.0f, float(THOOM_SCREEN_WIDTH), 11.0f};
    thoom::Renderer::instance->draw_rect(Hud::instance->ui, &black_rect,
                                         thoom::Colour(0, 0, 0),
                                         SDL_BLENDMODE_NONE);
    thoom::Renderer::instance->draw_text(
        Hud::instance->ui, MONO_FONT,
        " h:write, j:tile, k:sheet, l:layer, x:export  ", 0, 0, 0, kBackground);
    char status_text[256];
    if (this->layer != COLLISION) {
      // foreground / background status text
      snprintf(status_text, sizeof(status_text), "%s: %s %d,%d [%d]",
               this->map_path.c_str(), this->layer == BACKGROUND ? "bg" : "fg",
               this->sel_x * this->map->tile_width,
               this->sel_y * this->map->tile_height,
               vec == nullptr ? 0 : int(vec->size()));
    } else {
      // collision layer status text
      snprintf(status_text, sizeof(status_text), "%s: %s %d,%d",
               this->map_path.c_str(), "collision",
               this->sel_x * this->map->tile_width,
               this->sel_y * this->map->tile_height);
    }
    black_rect = {0.0f, float(THOOM_SCREEN_HEIGHT - 11),
                  float(THOOM_SCREEN_WIDTH), 11.0f};
    thoom::Renderer::instance->draw_rect(Hud::instance->ui, &black_rect,
                                         thoom::Colour(0, 0, 0),
                                         SDL_BLENDMODE_NONE);
    thoom::Renderer::instance->draw_text(
        Hud::instance->ui, MONO_FONT, std::string(status_text), 0,
        THOOM_SCREEN_HEIGHT - 11, 0, kBackground);

    switch (thoom::Input::instance->world.character) {
      case 'h':
        this->map->write(this->map_path.c_str());
        break;
      case 'j':
        if (this->sel_tilesheet < 0) break;

        this->user_inputting = true;
        this->input_mode = INPUT_TILE;
        this->text = "";
        break;
      case 'k':
        if (this->sel_tilesheet < 0) break;

        this->user_inputting = true;
        this->input_mode = INPUT_SHEET;
        this->text = "";
        break;
      case 'l':
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
        break;
      case 'x': {
        std::cout << "exporting map to mapdump.bmp" << std::endl;
        SDL_Surface* _bg = thoom::Window::instance->read_pixels(
            this->map->layers[thoom::Map::kBackgroundLayer]->texture);
        SDL_SaveBMP(_bg, "mapdump.bmp");
        SDL_DestroySurface(_bg);
      } break;
      default:
        break;
    }
    thoom::Input::instance->world.character = '\0';

    if (thoom::Input::instance->world.consume_hit(thoom::Button::ATTACK) &&
        this->sel_tilesheet >= 0 && this->sel_ts2_x >= 0) {
      if (this->layer != COLLISION) {
        int _x = THOOM_MIN(this->sel_ts_x, this->sel_ts2_x);
        int _y = THOOM_MIN(this->sel_ts_y, this->sel_ts2_y);
        int _w = THOOM_ABS(this->sel_ts_x - this->sel_ts2_x);
        int _h = THOOM_ABS(this->sel_ts_y - this->sel_ts2_y);

        for (int dy = 0; dy <= _h; dy++) {
          for (int dx = 0; dx <= _w; dx++) {
            if (this->sel_x + dx >= this->map->cols) {
              continue;
            }

            if (this->sel_y + dy >= this->map->rows) {
              break;
            }

            int _coord = coord + dx + (this->map->cols * dy);
            std::vector<thoom::Tile>* _vec;
            if (this->layer == FOREGROUND) {
              _vec = this->map->layers[thoom::Map::kForegroundLayer]
                         ->tiles[_coord];
              if (_vec == nullptr) {
                _vec = new std::vector<thoom::Tile>();
                this->map->layers[thoom::Map::kForegroundLayer]->tiles[_coord] =
                    _vec;
              }
            } else {
              _vec = this->map->layers[thoom::Map::kBackgroundLayer]
                         ->tiles[_coord];
              if (_vec == nullptr) {
                _vec = new std::vector<thoom::Tile>();
                this->map->layers[thoom::Map::kBackgroundLayer]->tiles[_coord] =
                    _vec;
              }
            }
            thoom::Tile tile;
            tile.tilesheet = this->sel_tilesheet;
            tile.x = _x + dx;
            tile.y = _y + dy;
            _vec->push_back(tile);
            this->map->render_tile(this->sel_x + dx, this->sel_y + dy);
          }
        }
      }
    } else if (thoom::Input::instance->world.consume_hit(thoom::Button::TOSS) &&
               this->sel_tilesheet >= 0 && this->sel_ts2_x >= 0) {
      int _x = THOOM_MIN(this->sel_ts_x, this->sel_ts2_x);
      int _y = THOOM_MIN(this->sel_ts_y, this->sel_ts2_y);
      int _w = (THOOM_ABS(this->sel_ts_x - this->sel_ts2_x) + 1) *
               this->map->tile_width;
      int _h = (THOOM_ABS(this->sel_ts_y - this->sel_ts2_y) + 1) *
               this->map->tile_height;
      char buff[1024];
      snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d,%d,%d,%s",
               this->sel_x * this->map->tile_width,
               this->sel_y * this->map->tile_height, _x, _y, _w, _h, _h,
               this->map->tilesheets[this->sel_tilesheet]->path.c_str());
      thoom::ObjectRequest object;
      object.id = "billboard";
      object.options = buff;
      this->map->objects.push_back(object);
      this->render_objects();
    } else if (thoom::Input::instance->world.down[thoom::Button::SELECT] &&
               this->sel_tilesheet >= 0) {
      if (this->layer == COLLISION) {
        this->map->collider->collision[coord] = this->sel_collider;
        this->render_collision_tile(this->collision_texture, this->sel_x,
                                    this->sel_y, this->sel_collider);
      } else {
        bool should_place = true;

        // place a tile
        if (vec == nullptr) {
          vec = new std::vector<thoom::Tile>();
          if (this->layer == FOREGROUND) {
            this->map->layers[thoom::Map::kForegroundLayer]->tiles[coord] = vec;
          } else {
            this->map->layers[thoom::Map::kBackgroundLayer]->tiles[coord] = vec;
          }
        } else {
          thoom::Tile top_tile = vec->back();
          if (top_tile.tilesheet == this->sel_tilesheet &&
              top_tile.x == this->sel_ts_x && top_tile.y == this->sel_ts_y)
            should_place = false;
        }

        if (should_place) {
          thoom::Tile new_tile;
          new_tile.tilesheet = this->sel_tilesheet;
          new_tile.x = this->sel_ts_x;
          new_tile.y = this->sel_ts_y;
          vec->push_back(new_tile);
          this->map->render_tile(this->sel_x, this->sel_y);
        }
      }
    } else if (thoom::Input::instance->world.consume_hit(
                   thoom::Button::CANCEL)) {
      if (this->layer == COLLISION) {
        this->map->collider->collision[coord] = -1;
        this->render_collision_tile(this->collision_texture, this->sel_x,
                                    this->sel_y, -1);
      } else {
        // remove a tile
        if (vec != nullptr) {
          vec->pop_back();
          if (vec->empty()) {
            delete vec;
            vec = nullptr;
            if (this->layer == FOREGROUND) {
              this->map->layers[thoom::Map::kForegroundLayer]->tiles[coord] =
                  nullptr;
            } else {
              this->map->layers[thoom::Map::kBackgroundLayer]->tiles[coord] =
                  nullptr;
            }
          }
          this->map->render_tile(this->sel_x, this->sel_y);
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
        thoom::Renderer::instance->clear(Hud::instance->ui, thoom::kMask);
        break;
    }
  }

  thoom::Renderer::instance->push_sprite("EDITOR", this->texture,
                                         &this->src_rect, &this->dst_rect, 0);
}

void Editor::input_tile() {
  // end inputting
  if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT) ||
      thoom::Input::instance->world.consume_hit(thoom::Button::DIGIT)) {
    this->user_inputting = false;
    thoom::Renderer::instance->clear(Hud::instance->ui, thoom::kMask);
    return;
  }

  // update selected collider or tile
  if (this->layer == COLLISION) {
    this->sel_collider += thoom::Input::instance->world.consume_hit_dx();
    int y_dir = thoom::Input::instance->world.consume_hit_dy();
    this->sel_collider += y_dir * 14;

    this->sel_collider =
        THOOM_CLAMP(this->sel_collider + 1, 0, thoom::n_MapColliders) - 1;
  } else {
    this->sel_ts_x += thoom::Input::instance->world.consume_hit_dx();
    this->sel_ts_y += thoom::Input::instance->world.consume_hit_dy();
    this->sel_ts_x =
        THOOM_CLAMP(this->sel_ts_x, 0,
                    this->map->tilesheets[this->sel_tilesheet]->cols - 1);
    this->sel_ts_y =
        THOOM_CLAMP(this->sel_ts_y, 0,
                    this->map->tilesheets[this->sel_tilesheet]->rows - 1);

    if (thoom::Input::instance->world.consume_hit(
            thoom::Button::ATTACK)) {  // space
      this->sel_ts2_x = this->sel_ts_x;
      this->sel_ts2_y = this->sel_ts_y;
    } else if (thoom::Input::instance->world.consume_hit(
                   thoom::Button::TOSS)) {  // c
      this->sel_ts2_x = this->sel_ts2_y = -1;
    }
  }

  SDL_FRect tilesheet_rect;
  tilesheet_rect.x = 0.0f;
  tilesheet_rect.y = 11.0f;

  if (this->layer == COLLISION) {
    tilesheet_rect.w = this->collision_sheet->w;
    tilesheet_rect.h = this->collision_sheet->h;
  } else {
    tilesheet_rect.w = this->map->tilesheets[this->sel_tilesheet]->texture->w;
    tilesheet_rect.h = this->map->tilesheets[this->sel_tilesheet]->texture->h;
  }

  thoom::Renderer::instance->draw_rect(this->texture, &tilesheet_rect,
                                       thoom::Colour(128, 128, 128),
                                       SDL_BLENDMODE_NONE);

  if (this->layer == COLLISION) {
    thoom::Renderer::instance->draw_texture(
        this->texture, this->collision_sheet, NULL, &tilesheet_rect);
  } else {
    thoom::Renderer::instance->draw_texture(
        this->texture, this->map->tilesheets[this->sel_tilesheet]->texture,
        NULL, &tilesheet_rect);
  }

  SDL_FRect sel_tile_rect;
  sel_tile_rect.w = this->map->tile_width;
  sel_tile_rect.h = this->map->tile_height;

  if (this->layer == COLLISION) {
    int _x = (this->sel_collider + 1) % 14;
    int _y = (this->sel_collider + 1) / 14;
    sel_tile_rect.x = this->map->tile_width * _x;
    sel_tile_rect.y = tilesheet_rect.y + (this->map->tile_height * _y);
  } else {
    sel_tile_rect.x = this->map->tile_width * this->sel_ts_x;
    sel_tile_rect.y =
        tilesheet_rect.y + this->map->tile_height * this->sel_ts_y;
  }

  thoom::Renderer::instance->draw_outline(this->texture, &sel_tile_rect,
                                          thoom::Colour{255, 128, 255, 240},
                                          SDL_BLENDMODE_BLEND);

  if (this->sel_ts2_x >= 0) {
    SDL_FRect sel_tile2_rect;
    sel_tile2_rect.w = this->map->tile_width;
    sel_tile2_rect.h = this->map->tile_height;
    sel_tile2_rect.x = this->map->tile_width * this->sel_ts2_x;
    sel_tile2_rect.y =
        tilesheet_rect.y + this->map->tile_height * this->sel_ts2_y;

    thoom::Renderer::instance->draw_outline(this->texture, &sel_tile2_rect,
                                            thoom::Colour{255, 255, 128, 240},
                                            SDL_BLENDMODE_BLEND);
  }

  SDL_FRect black_rect = {0.0f, 0.0f, float(THOOM_SCREEN_WIDTH), 11.0f};
  thoom::Renderer::instance->draw_rect(Hud::instance->ui, &black_rect,
                                       thoom::Colour(0, 0, 0),
                                       SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_text(Hud::instance->ui, MONO_FONT,
                                       "<Return> to place, <0-9> to close", 0,
                                       0, 0, kBackground);
}

void Editor::input_sheet() {
  int sel_i, nfields;
  nfields = sscanf(this->text.c_str(), "%d", &sel_i);

  if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
    if (text.empty()) {
      // close
      this->user_inputting = false;
      thoom::Renderer::instance->clear(Hud::instance->ui, thoom::kMask);
      return;
    } else if (nfields == 1 && sel_i < this->map->tilesheets.size()) {
      this->sel_tilesheet = sel_i;
    }
  }

  if (thoom::Input::instance->world.consume_hit(thoom::Button::CANCEL) &&
      !text.empty())
    this->text.pop_back();
  else if (thoom::Input::instance->world.character != '\0')
    this->text.push_back(thoom::Input::instance->world.character);

  std::string display_text = "Tilesheet: " + this->text +
                             "\n<Return> to select or close (when empty)\n\n";
  for (int i = 0; i < this->map->tilesheets.size(); i++) {
    char tilesheet[256];
    char first_c, second_c,
        end_char = i == this->map->tilesheets.size() - 1 ? ' ' : '\n';

    if (nfields == 1 && i == sel_i)
      first_c = '*';
    else
      first_c = ' ';
    if (i == this->sel_tilesheet)
      second_c = '*';
    else
      second_c = ' ';

    snprintf(tilesheet, 256, "%c%c%d: %s%c", first_c, second_c, i,
             this->map->tilesheets[i]->path.c_str(), end_char);
    display_text += tilesheet;
  }
  SDL_FRect black_rect = {0.0f, 0.0f, float(THOOM_SCREEN_WIDTH),
                          float(THOOM_SCREEN_HEIGHT)};
  thoom::Renderer::instance->draw_rect(Hud::instance->ui, &black_rect,
                                       thoom::Colour(0, 0, 0),
                                       SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_text(Hud::instance->ui, MONO_FONT,
                                       display_text, 0, 0, 0, kBackground);
}

void Editor::render() {
  SDL_FRect src, dst;

  int x = (this->sel_x * this->map->tile_width) + (this->map->tile_width / 2);
  int y = (this->sel_y * this->map->tile_height) + (this->map->tile_height / 2);

  thoom::Map::make_map_rect(
      x - THOOM_SCREEN_WIDTH / 2, y - THOOM_SCREEN_HEIGHT / 2,
      this->map->tile_width * this->map->cols,
      this->map->tile_height * this->map->rows, &src, &dst);

  SDL_FRect selected_tile_rect;
  selected_tile_rect.x =
      (float)(int)((THOOM_SCREEN_WIDTH / 2) - (this->map->tile_width / 2));
  selected_tile_rect.y =
      (float)(int)((THOOM_SCREEN_HEIGHT / 2) - (this->map->tile_height / 2));
  selected_tile_rect.w = float(this->map->tile_width);
  selected_tile_rect.h = float(this->map->tile_height);

  thoom::Renderer::instance->clear(this->texture, thoom::Colour(128, 128, 128));
  thoom::Renderer::instance->draw_texture(
      this->texture, this->map->layers[thoom::Map::kBackgroundLayer]->texture,
      &src, &dst);

  // only render the foreground if not only background
  if (this->layer != BACKGROUND)
    thoom::Renderer::instance->draw_texture(
        this->texture, this->map->layers[thoom::Map::kForegroundLayer]->texture,
        &src, &dst);
  else {
    // render foreground at half opacity if viewing background
    SDL_SetTextureAlphaMod(
        this->map->layers[thoom::Map::kForegroundLayer]->texture, 0x80);
    thoom::Renderer::instance->draw_texture(
        this->texture, this->map->layers[thoom::Map::kForegroundLayer]->texture,
        &src, &dst);
    SDL_SetTextureAlphaMod(
        this->map->layers[thoom::Map::kForegroundLayer]->texture, 0xff);
  }

  // render collision layer if that mode is selected
  if (this->layer == COLLISION) {
    thoom::Renderer::instance->draw_texture(this->texture, objects, &src, &dst);
    thoom::Renderer::instance->draw_texture(
        this->texture, this->collision_texture, &src, &dst);
  }

  thoom::Renderer::instance->draw_outline(this->texture, &selected_tile_rect,
                                          thoom::Colour{255, 128, 255, 240},
                                          SDL_BLENDMODE_BLEND);

  if (this->sel_tilesheet >= 0) {
    selected_tile_rect.x = 0.0f;
    selected_tile_rect.y =
        float(THOOM_SCREEN_HEIGHT - 11 - this->map->tile_height);
    thoom::Renderer::instance->draw_rect(this->texture, &selected_tile_rect,
                                         thoom::Colour(128, 128, 128),
                                         SDL_BLENDMODE_NONE);
    SDL_FRect selected_tile_src_rect;
    selected_tile_src_rect.w = float(this->map->tile_width);
    selected_tile_src_rect.h = float(this->map->tile_height);

    if (this->layer == COLLISION) {
      int _x = (this->sel_collider + 1) % 14;
      int _y = (this->sel_collider + 1) / 14;
      selected_tile_src_rect.x = float(_x * this->map->tile_width);
      selected_tile_src_rect.y = float(_y * this->map->tile_height);
      thoom::Renderer::instance->draw_texture(
          this->texture, this->collision_sheet, &selected_tile_src_rect,
          &selected_tile_rect);
    } else {
      selected_tile_src_rect.x = float(this->sel_ts_x * this->map->tile_width);
      selected_tile_src_rect.y = float(this->sel_ts_y * this->map->tile_height);
      thoom::Renderer::instance->draw_texture(
          this->texture, this->map->tilesheets[this->sel_tilesheet]->texture,
          &selected_tile_src_rect, &selected_tile_rect);
    }
  }
}

void Editor::render_collision_tile(SDL_Texture* target, int x, int y,
                                   int collider) {
  SDL_FRect dst_rect;
  dst_rect.x = float(this->map->tile_width * x);
  dst_rect.y = float(this->map->tile_height * y);
  dst_rect.w = float(this->map->tile_width);
  dst_rect.h = float(this->map->tile_height);

  // render transparency to clear the tile
  thoom::Renderer::instance->draw_rect(target, &dst_rect, thoom::kMask,
                                       SDL_BLENDMODE_NONE);

  if (collider < 0) {
    return;  // all done
  }

  // render the collision quad
  SDL_Vertex vertices[4];
  // translate the vertices to match the destination rect
  for (int i = 0; i < 4; i++) {
    vertices[i].color.r = 1.0f;
    vertices[i].color.g = 1.0f;
    vertices[i].color.b = 1.0f;
    vertices[i].color.a = 0.5f;
    vertices[i].position.x =
        dst_rect.x + this->map->collider->quads[collider].vertex[i].x;
    vertices[i].position.y =
        dst_rect.y + this->map->collider->quads[collider].vertex[i].y;
  }
  const int indices[6] = {0, 1, 2, 2, 3, 0};
  thoom::Renderer::instance->draw_geometry(target, nullptr, vertices, 4,
                                           indices, 6);
}

static void _ParseBillboardOptions(const std::string& options, int* x, int* y,
                                   int* ts_x, int* ts_y, int* w, int* h,
                                   int* depth, std::string& tilesheet) {
  char buff[1024];
  if (sscanf(options.c_str(), "%d,%d,%d,%d,%d,%d,%d,%1023[^\n]", x, y, ts_x,
             ts_y, w, h, depth, buff) < 8) {
    std::cerr << "_ParseBillboardOptions error: bad options: " << options
              << std::endl;
    std::exit(1);
  }

  tilesheet = buff;
}

void Editor::render_objects() {
  thoom::Renderer::instance->clear(this->objects, thoom::kMask);

  for (const thoom::ObjectRequest& object : this->map->objects) {
    if (object.id == "billboard") {
      int x, y, ts_x, ts_y, w, h, depth;
      std::string tilesheet;

      _ParseBillboardOptions(object.options, &x, &y, &ts_x, &ts_y, &w, &h,
                             &depth, tilesheet);

      SDL_Texture* tex = thoom::Texture::instance->load(tilesheet);

      SDL_FRect src_rect;
      src_rect.x = ts_x * this->map->tile_width;
      src_rect.y = ts_y * this->map->tile_height;
      src_rect.w = w;
      src_rect.h = h;
      SDL_FRect dst_rect = SDL_FRect{float(x), float(y), float(w), float(h)};

      SDL_SetTextureAlphaMod(tex, 0x40);
      thoom::Renderer::instance->draw_texture(this->objects, tex, &src_rect,
                                              &dst_rect);
      SDL_SetTextureAlphaMod(tex, 0xff);

      thoom::Renderer::instance->draw_outline(this->objects, &dst_rect,
                                              thoom::Colour(255, 128, 0),
                                              SDL_BLENDMODE_NONE);
    } else {
      int x, y;
      if (sscanf(object.options.c_str(), "%d,%d", &x, &y) == 2) {
        SDL_FRect dst_rect =
            SDL_FRect{float(x) - 8.0f, float(y) - 8.0f, 16.0f, 16.0f};
        draw_icon(EDITOR_OBJECT_ICON, this->objects, &dst_rect);
        thoom::Renderer::instance->draw_text(
            this->objects, SMALL_FONT, object.id, x + 6, y - 4, 0, kBackground);
      }
    }
  }
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
