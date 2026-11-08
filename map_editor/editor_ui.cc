#include "editor_ui.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <string>

#include "colour.h"
#include "hud/hud.h"
#include "input/controller.h"
#include "input/input.h"
#include "map/map.h"
#include "map/map_collider.h"
#include "map/tilesheet.h"
#include "renderer/font.h"
#include "renderer/renderer.h"
#include "texture/texture.h"
#include "thoom/macros.h"
#include "window/window.h"

EditorUI::~EditorUI() {
  if (this->collision_sheet != nullptr) {
    SDL_DestroyTexture(this->collision_sheet);
    this->collision_sheet = nullptr;
  }
}

void EditorUI::init(thoom::Map* map) {
  if (!map->tilesheets.empty()) {
    this->tilesheet = 0;
  }

  this->tile_x = map->cols / 2;
  this->tile_y = map->rows / 2;

  int count = int(thoom::n_MapColliders) + 1;
  int rows = (count + this->sheet_cols - 1) / this->sheet_cols;

  this->collision_sheet = thoom::Texture::instance->create_texture(
      map->tile_width * this->sheet_cols, map->tile_height * rows,
      SDL_PIXELFORMAT_RGBA8888, SDL_SCALEMODE_LINEAR);

  thoom::Renderer::instance->clear(this->collision_sheet, thoom::kMask);

  SDL_FRect sheet_rect;
  sheet_rect.x = 0.0f;
  sheet_rect.y = 0.0f;
  sheet_rect.w = float(map->tile_width * this->sheet_cols);
  sheet_rect.h = float(map->tile_height * rows);
  thoom::Renderer::instance->draw_collision_sheet(
      this->collision_sheet, map, &sheet_rect, this->sheet_cols,
      thoom::Colour(255, 255, 255, 128));
}

void EditorUI::move(const thoom::Map* map) {
  this->tile_x += thoom::Input::instance->world.consume_hit_dx();
  this->tile_y += thoom::Input::instance->world.consume_hit_dy();
  this->tile_x = THOOM_CLAMP(this->tile_x, 0, map->cols - 1);
  this->tile_y = THOOM_CLAMP(this->tile_y, 0, map->rows - 1);
}

void EditorUI::step(const thoom::Map* map, EditorSelectedLayer layer) {
  switch (this->input_mode) {
    case INPUT_TILE:
      this->step_tile(map, layer);
      break;
    case INPUT_SHEET:
      this->step_sheet(map);
      break;
    default:
      this->inputting = false;
      break;
  }
}

void EditorUI::render(SDL_Texture* dst, const thoom::Map* map,
                      EditorSelectedLayer layer) const {
  SDL_FRect selected_tile_rect;
  selected_tile_rect.x =
      float((THOOM_SCREEN_WIDTH / 2) - (map->tile_width / 2));
  selected_tile_rect.y =
      float((THOOM_SCREEN_HEIGHT / 2) - (map->tile_height / 2));
  selected_tile_rect.w = float(map->tile_width);
  selected_tile_rect.h = float(map->tile_height);

  thoom::Renderer::instance->draw_outline(dst, &selected_tile_rect,
                                          thoom::Colour{255, 128, 255, 240},
                                          SDL_BLENDMODE_BLEND);

  if (layer != COLLISION &&
      (this->tilesheet < 0 || this->tilesheet >= int(map->tilesheets.size()))) {
    return;
  }

  SDL_FRect preview_rect;
  preview_rect.x = 0.0f;
  preview_rect.y = float(THOOM_SCREEN_HEIGHT - 11 - map->tile_height);
  preview_rect.w = float(map->tile_width);
  preview_rect.h = float(map->tile_height);

  thoom::Renderer::instance->draw_rect(
      dst, &preview_rect, thoom::Colour(128, 128, 128), SDL_BLENDMODE_NONE);

  SDL_FRect src_rect;
  src_rect.w = float(map->tile_width);
  src_rect.h = float(map->tile_height);

  SDL_Texture* texture = nullptr;

  if (layer == COLLISION) {
    int index = this->collider + 1;
    src_rect.x = float((index % this->sheet_cols) * map->tile_width);
    src_rect.y = float((index / this->sheet_cols) * map->tile_height);
    texture = this->collision_sheet;
  } else {
    src_rect.x = float(this->sheet_x * map->tile_width);
    src_rect.y = float(this->sheet_y * map->tile_height);
    texture = map->tilesheets[this->tilesheet]->texture;
  }

  thoom::Renderer::instance->draw_texture(dst, texture, &src_rect,
                                          &preview_rect);
}

void EditorUI::render_input(SDL_Texture* dst, const thoom::Map* map,
                            EditorSelectedLayer layer) const {
  switch (this->input_mode) {
    case INPUT_TILE:
      this->render_tile_input(dst, map, layer);
      break;
    case INPUT_SHEET:
      this->render_sheet_input(map);
      break;
    default:
      break;
  }
}

void EditorUI::begin_tile() {
  this->inputting = true;
  this->input_mode = INPUT_TILE;
  this->text.clear();
}

void EditorUI::begin_sheet() {
  this->inputting = true;
  this->input_mode = INPUT_SHEET;
  this->text.clear();
}

void EditorUI::step_tile(const thoom::Map* map, EditorSelectedLayer layer) {
  if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT) ||
      thoom::Input::instance->world.consume_hit(thoom::Button::DIGIT)) {
    this->inputting = false;
    return;
  }

  if (layer == COLLISION) {
    int dx = thoom::Input::instance->world.consume_hit_dx();
    int dy = thoom::Input::instance->world.consume_hit_dy();
    this->collider += dx + (dy * this->sheet_cols);
    this->collider =
        THOOM_CLAMP(this->collider + 1, 0, int(thoom::n_MapColliders)) - 1;
    return;
  }

  if (this->tilesheet < 0 || this->tilesheet >= int(map->tilesheets.size())) {
    this->inputting = false;
    return;
  }

  thoom::Tilesheet* tilesheet = map->tilesheets[this->tilesheet];

  this->sheet_x += thoom::Input::instance->world.consume_hit_dx();
  this->sheet_y += thoom::Input::instance->world.consume_hit_dy();
  this->sheet_x = THOOM_CLAMP(this->sheet_x, 0, tilesheet->cols - 1);
  this->sheet_y = THOOM_CLAMP(this->sheet_y, 0, tilesheet->rows - 1);

  if (thoom::Input::instance->world.consume_hit(thoom::Button::ATTACK)) {
    this->region_x = this->sheet_x;
    this->region_y = this->sheet_y;
  } else if (thoom::Input::instance->world.consume_hit(thoom::Button::TOSS)) {
    this->region_x = -1;
    this->region_y = -1;
  }
}

void EditorUI::step_sheet(const thoom::Map* map) {
  int sel_i = 0;
  int nfields = sscanf(this->text.c_str(), "%d", &sel_i);

  if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
    if (this->text.empty()) {
      this->inputting = false;
      return;
    }

    if (nfields == 1 && sel_i >= 0 && sel_i < int(map->tilesheets.size())) {
      this->tilesheet = sel_i;
    }
  }

  if (thoom::Input::instance->world.consume_hit(thoom::Button::CANCEL) &&
      !this->text.empty()) {
    this->text.pop_back();
  } else if (thoom::Input::instance->world.character != '\0') {
    this->text.push_back(thoom::Input::instance->world.character);
  }
}

void EditorUI::render_tile_input(SDL_Texture* dst, const thoom::Map* map,
                                 EditorSelectedLayer layer) const {
  SDL_Texture* texture = nullptr;

  if (layer == COLLISION) {
    texture = this->collision_sheet;
  } else {
    if (this->tilesheet < 0 || this->tilesheet >= int(map->tilesheets.size())) {
      return;
    }
    texture = map->tilesheets[this->tilesheet]->texture;
  }

  SDL_FRect tilesheet_rect;
  tilesheet_rect.x = 0.0f;
  tilesheet_rect.y = 11.0f;
  tilesheet_rect.w = float(texture->w);
  tilesheet_rect.h = float(texture->h);

  thoom::Renderer::instance->draw_rect(
      dst, &tilesheet_rect, thoom::Colour(128, 128, 128), SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_texture(dst, texture, nullptr,
                                          &tilesheet_rect);

  SDL_FRect selected_tile_rect;
  selected_tile_rect.w = float(map->tile_width);
  selected_tile_rect.h = float(map->tile_height);

  if (layer == COLLISION) {
    int index = this->collider + 1;
    selected_tile_rect.x = float((index % this->sheet_cols) * map->tile_width);
    selected_tile_rect.y =
        tilesheet_rect.y + float((index / this->sheet_cols) * map->tile_height);
  } else {
    selected_tile_rect.x = float(this->sheet_x * map->tile_width);
    selected_tile_rect.y =
        tilesheet_rect.y + float(this->sheet_y * map->tile_height);
  }

  thoom::Renderer::instance->draw_outline(dst, &selected_tile_rect,
                                          thoom::Colour{255, 128, 255, 240},
                                          SDL_BLENDMODE_BLEND);

  if (this->region_x >= 0 && layer != COLLISION) {
    SDL_FRect region_rect;
    region_rect.x = float(this->region_x * map->tile_width);
    region_rect.y = tilesheet_rect.y + float(this->region_y * map->tile_height);
    region_rect.w = float(map->tile_width);
    region_rect.h = float(map->tile_height);

    thoom::Renderer::instance->draw_outline(dst, &region_rect,
                                            thoom::Colour{255, 255, 128, 240},
                                            SDL_BLENDMODE_BLEND);
  }

  SDL_FRect black_rect = {0.0f, 0.0f, float(THOOM_SCREEN_WIDTH), 11.0f};
  thoom::Renderer::instance->draw_rect(thoom::Hud::instance->ui, &black_rect,
                                       thoom::Colour(0, 0, 0),
                                       SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_text(thoom::Hud::instance->ui, MONO_FONT,
                                       "<Return> to place, <0-9> to close", 0,
                                       0, 0, kBackground);
}

void EditorUI::render_sheet_input(const thoom::Map* map) const {
  int sel_i = 0;
  int nfields = sscanf(this->text.c_str(), "%d", &sel_i);

  std::string display_text = "Tilesheet: " + this->text +
                             "\n<Return> to select or close (when empty)\n\n";

  for (int i = 0; i < int(map->tilesheets.size()); i++) {
    char tilesheet[256];
    char first_c = nfields == 1 && i == sel_i ? '*' : ' ';
    char second_c = i == this->tilesheet ? '*' : ' ';
    char end_char = i == int(map->tilesheets.size()) - 1 ? ' ' : '\n';

    snprintf(tilesheet, sizeof(tilesheet), "%c%c%d: %s%c", first_c, second_c, i,
             map->tilesheets[i]->path.c_str(), end_char);
    display_text += tilesheet;
  }

  SDL_FRect black_rect = {0.0f, 0.0f, float(THOOM_SCREEN_WIDTH),
                          float(THOOM_SCREEN_HEIGHT)};
  thoom::Renderer::instance->draw_rect(thoom::Hud::instance->ui, &black_rect,
                                       thoom::Colour(0, 0, 0),
                                       SDL_BLENDMODE_NONE);
  thoom::Renderer::instance->draw_text(thoom::Hud::instance->ui, MONO_FONT,
                                       display_text, 0, 0, 0,
                                       kBackground);
}
