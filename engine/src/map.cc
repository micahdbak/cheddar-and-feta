#include "map.h"

#include <iostream>

#include "game.h"

Tilesheet::Tilesheet(const char* tilesheet_path, int tile_width,
                     int tile_height) {
  SDL_Surface* tilesheet_surface = SDL_LoadBMP(tilesheet_path);
  if (tilesheet_surface == nullptr) {
    std::cerr << "Tilesheet::Tilesheet error: '" << tilesheet_path
              << "' does not exist." << std::endl;
    this->texture = nullptr;
    this->cols = 0;
    this->rows = 0;
    return;
  }

  this->cols = tilesheet_surface->w / tile_width;
  this->rows = tilesheet_surface->h / tile_height;

  this->texture = SDL_CreateTextureFromSurface(renderer, tilesheet_surface);
  SDL_DestroySurface(tilesheet_surface);
  tilesheet_surface = nullptr;

  this->path = tilesheet_path;
}

Tilesheet::~Tilesheet() {
  SDL_DestroyTexture(this->texture);
  this->texture = nullptr;
}

Map::~Map() { this->clear(); }

#define CORRUPTED_EXIT                                         \
  {                                                            \
    std::cerr << "map.cpp (" << __LINE__ << "): '" << map_path \
              << "' corrupted." << std::endl;                  \
    exit(1);                                                   \
  }

void Map::make_empty(int tile_width, int tile_height, int cols, int rows) {
  this->clear();

  size_t nbytes = sizeof(std::vector<Tile>*) * cols * rows;
  this->bg_tiles = (std::vector<Tile>**)malloc(nbytes);
  this->fg_tiles = (std::vector<Tile>**)malloc(nbytes);
  this->collision = (int*)malloc(sizeof(int) * cols * rows);

  // zero the allocated memory
  memset(this->bg_tiles, 0, nbytes);
  memset(this->fg_tiles, 0, nbytes);
  for (int i = 0; i < cols * rows; i++) {
    this->collision[i] = -1;
  }

  this->tile_width = tile_width;
  this->tile_height = tile_height;
  this->cols = cols;
  this->rows = rows;

  this->bg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                               SDL_TEXTUREACCESS_TARGET, cols * tile_width,
                               rows * tile_height);
  this->fg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                               SDL_TEXTUREACCESS_TARGET, cols * tile_width,
                               rows * tile_height);

  if (this->bg == nullptr || this->fg == nullptr) {
    std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
    exit(1);
  }

  SDL_SetRenderTarget(renderer, this->bg);
  SDL_SetRenderDrawColor(renderer, this->bg_r, this->bg_g, this->bg_b, 255);
  SDL_RenderFillRect(renderer, NULL);
  SDL_SetRenderTarget(renderer, this->fg);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_RenderFillRect(renderer, NULL);
  SDL_SetRenderTarget(renderer, game->screen);
}

void Map::read(const char* map_path) {
  std::string map_txt_path = std::string(map_path) + ".txt";
  std::string map_bin_path = std::string(map_path) + ".bin";

  FILE* txt_file = fopen(map_txt_path.c_str(), "r");
  if (txt_file == NULL) CORRUPTED_EXIT
  char buff[1024], buff2[1024];

  if (feof(txt_file)) CORRUPTED_EXIT;
  fgets(buff, sizeof(buff), txt_file);
  sscanf(buff, "%1023[^\n]", buff2);
  buff2[1023] = '\0';
  this->title = buff2;

  if (feof(txt_file)) CORRUPTED_EXIT;
  fgets(buff, sizeof(buff), txt_file);
  sscanf(buff, "%1023[^\n]", buff2);
  buff2[1023] = '\0';
  this->description = buff2;

  if (feof(txt_file)) CORRUPTED_EXIT;
  fgets(buff, sizeof(buff), txt_file);
  if (4 != sscanf(buff, "%d,%d,%d,%d", &this->tile_width, &this->tile_height,
                  &this->cols, &this->rows))
    CORRUPTED_EXIT
  if (tile_width <= 0 || tile_height <= 0) CORRUPTED_EXIT

  if (feof(txt_file)) CORRUPTED_EXIT;
  fgets(buff, sizeof(buff), txt_file);
  if (3 != sscanf(buff, "%d,%d,%d", &this->bg_r, &this->bg_g, &this->bg_b))
    CORRUPTED_EXIT

  if (feof(txt_file)) CORRUPTED_EXIT;
  fgets(buff, sizeof(buff), txt_file);
  buff[strlen(buff) - 1] = '\0';  // remove newline
  this->ambience = buff;

  // allocate the necessary memory
  this->make_empty(this->tile_width, this->tile_height, this->cols, this->rows);

  if (feof(txt_file)) CORRUPTED_EXIT;
  fgets(buff, sizeof(buff), txt_file);
  int ntilesheets = 0;
  if (1 != sscanf(buff, "%d", &ntilesheets)) CORRUPTED_EXIT

  for (int i = 0; i < ntilesheets; i++) {
    if (feof(txt_file)) CORRUPTED_EXIT;
    fgets(buff, sizeof(buff), txt_file);
    sscanf(buff, "%1023[^\n]", buff2);
    buff2[1023] = '\0';
    Tilesheet* tilesheet =
        new Tilesheet(buff2, this->tile_width, this->tile_height);
    if (tilesheet->texture == nullptr) CORRUPTED_EXIT
    this->tilesheets.push_back(tilesheet);
  }

  if (feof(txt_file)) CORRUPTED_EXIT;
  fgets(buff, sizeof(buff), txt_file);
  int nobjects = 0;
  if (1 != sscanf(buff, "%d", &nobjects)) CORRUPTED_EXIT

  for (int i = 0; i < nobjects; i++) {
    if (feof(txt_file)) CORRUPTED_EXIT;
    fgets(buff, sizeof(buff), txt_file);
    char buff3[1024];
    if (2 != sscanf(buff, "%1023s %1023[^\n]", buff2, buff3)) CORRUPTED_EXIT
    buff2[1023] = buff3[1023] = '\0';
    this->objects.push_back(std::pair<std::string, std::string>(buff2, buff3));
  }

  fclose(txt_file);
  FILE* bin_file = fopen(map_bin_path.c_str(), "rb");
  if (bin_file == NULL) CORRUPTED_EXIT
  uint8_t buffer[6];
  size_t nbytes;

  // read tiles
  while (fread(buffer, 1, 6, bin_file) == 6) {
    int x = int(buffer[3]);
    int y = int(buffer[4]);
    int coord = (y * this->cols) + x;
    if (coord >= this->cols * this->rows) CORRUPTED_EXIT

    // collider
    if (buffer[5] == 'c') {
      // make sure the collider actually exists
      if (int(buffer[0]) >= n_MapColliders) CORRUPTED_EXIT

      this->collision[coord] = int(buffer[0]);
      // buffer[1] and buffer[2] are redundant
      continue;
    }

    Tile tile;
    tile.tilesheet = int(buffer[0]);
    tile.x = int(buffer[1]);
    tile.y = int(buffer[2]);

    // make sure nothing is corrupted
    if (tile.tilesheet >= this->tilesheets.size() ||
        tile.x >= this->tilesheets[tile.tilesheet]->cols ||
        tile.y >= this->tilesheets[tile.tilesheet]->rows)
      CORRUPTED_EXIT

    // background or foreground tile
    if (buffer[5] == 'b') {
      std::vector<Tile>* vec = this->bg_tiles[coord];
      if (vec == nullptr) {
        vec = new std::vector<Tile>();
        this->bg_tiles[coord] = vec;
      }
      vec->push_back(tile);
    } else if (buffer[5] == 'f') {
      std::vector<Tile>* vec = this->fg_tiles[coord];
      if (vec == nullptr) {
        vec = new std::vector<Tile>();
        this->fg_tiles[coord] = vec;
      }
      vec->push_back(tile);
    } else
      CORRUPTED_EXIT
  }

  // done reading file
  fclose(bin_file);

  // render all tiles read from the file
  for (int i = 0; i < this->cols * this->rows; i++) {
    render_tile(i % this->cols, i / this->cols);
  }

  SDL_SetRenderTarget(renderer, game->screen);
}

static inline void write_tile(uint8_t* buffer, Tile tile, int x, int y,
                              char ground, FILE* file) {
  buffer[0] = uint8_t(tile.tilesheet);
  buffer[1] = uint8_t(tile.x);
  buffer[2] = uint8_t(tile.y);
  buffer[3] = uint8_t(x);
  buffer[4] = uint8_t(y);
  buffer[5] = uint8_t(ground);
  fwrite(buffer, 1, 6, file);
}

void Map::write(const char* map_path) {
  std::string map_txt_path = std::string(map_path) + ".txt";
  std::string map_bin_path = std::string(map_path) + ".bin";

  FILE* txt_file = fopen(map_txt_path.c_str(), "w");
  if (txt_file == nullptr) CORRUPTED_EXIT

  fprintf(txt_file, "%s\n%s\n", this->title.c_str(), this->description.c_str());
  fprintf(txt_file, "%d,%d,%d,%d\n", this->tile_width, this->tile_height,
          this->cols, this->rows);
  fprintf(txt_file, "%d,%d,%d\n", this->bg_r, this->bg_g, this->bg_b);
  fprintf(txt_file, "%s\n", this->ambience.c_str());
  fprintf(txt_file, "%d\n", int(this->tilesheets.size()));

  // write tilesheet info
  for (Tilesheet* tilesheet : this->tilesheets) {
    fprintf(txt_file, "%s\n", tilesheet->path.c_str());
  }

  fprintf(txt_file, "%d\n", int(this->objects.size()));

  // write objects
  for (auto& obj : this->objects) {
    fprintf(txt_file, "%s %s\n", obj.first.c_str(), obj.second.c_str());
  }

  fclose(txt_file);
  FILE* bin_file = fopen(map_bin_path.c_str(), "wb");
  if (bin_file == nullptr) CORRUPTED_EXIT;
  uint8_t buffer[1024];

  // write tiles
  for (int i = 0; i < this->cols * this->rows; i++) {
    int x = i % this->cols, y = i / this->cols;
    // colliders
    int c = this->collision[i];
    if (c >= 0) {
      buffer[0] = uint8_t(c);
      buffer[1] = 'c';
      buffer[2] = 'c';
      buffer[3] = uint8_t(x);
      buffer[4] = uint8_t(y);
      buffer[5] = uint8_t('c');
      fwrite(buffer, 1, 6, bin_file);
    }

    // background tiles
    std::vector<Tile>* vec = this->bg_tiles[i];
    if (vec != nullptr)
      for (auto tile : *vec) write_tile(buffer, tile, x, y, 'b', bin_file);

    // foreground tiles
    vec = this->fg_tiles[i];
    if (vec != nullptr)
      for (auto tile : *vec) write_tile(buffer, tile, x, y, 'f', bin_file);
  }

  fclose(bin_file);
}

void Map::clear() {
  for (Tilesheet* tilesheet : this->tilesheets) delete tilesheet;
  this->tilesheets.clear();
  if (this->bg_tiles != nullptr) {
    for (int coord = 0; coord < this->rows * this->cols; coord++) {
      std::vector<Tile>* vec = this->bg_tiles[coord];
      if (vec != nullptr) delete vec;
    }

    free(this->bg_tiles);
    this->bg_tiles = nullptr;
  }
  if (this->fg_tiles != nullptr) {
    for (int coord = 0; coord < this->rows * this->cols; coord++) {
      std::vector<Tile>* vec = this->fg_tiles[coord];
      if (vec != nullptr) delete vec;
    }

    free(this->fg_tiles);
    this->fg_tiles = nullptr;
  }
  this->objects.clear();
}

void Map::render_tile(int x, int y) {
  SDL_FRect dst_rect;
  dst_rect.x = float(this->tile_width * x);
  dst_rect.y = float(this->tile_width * y);
  dst_rect.w = float(this->tile_width);
  dst_rect.h = float(this->tile_height);

  SDL_SetRenderTarget(renderer, this->bg);
  SDL_SetRenderDrawColor(renderer, this->bg_r, this->bg_g, this->bg_b, 255);
  SDL_RenderFillRect(renderer, &dst_rect);  // fill a black square

  // render bg tiles
  std::vector<Tile>* vec = this->bg_tiles[(y * this->cols) + x];
  if (vec != nullptr) {
    SDL_FRect src_rect;
    src_rect.w = float(this->tile_width);
    src_rect.h = float(this->tile_height);

    // render all tiles in order
    for (auto tile : *vec) {
      src_rect.x = float(this->tile_width * tile.x);
      src_rect.y = float(this->tile_height * tile.y);
      SDL_RenderTexture(renderer, this->tilesheets[tile.tilesheet]->texture,
                        &src_rect, &dst_rect);
    }
  }

  SDL_SetRenderTarget(renderer, this->fg);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_RenderFillRect(renderer, &dst_rect);  // fill a transparent square

  // render fg tiles
  vec = this->fg_tiles[(y * this->cols) + x];
  if (vec != nullptr) {
    SDL_FRect src_rect;
    src_rect.w = float(this->tile_width);
    src_rect.h = float(this->tile_height);

    // render all tiles in order
    for (auto tile : *vec) {
      src_rect.x = float(this->tile_width * tile.x);
      src_rect.y = float(this->tile_height * tile.y);
      SDL_RenderTexture(renderer, this->tilesheets[tile.tilesheet]->texture,
                        &src_rect, &dst_rect);
    }
  }
}
