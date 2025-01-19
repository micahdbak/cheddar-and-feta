#include "game.h"
#include "map.h"

#include <iostream>

Tilesheet::Tilesheet(const char *tilesheet_path, int tile_width, int tile_height) {
    SDL_Surface *tilesheet_surface = SDL_LoadBMP(tilesheet_path);
    if (tilesheet_surface == nullptr) {
        std::cerr << "Tilesheet::Tilesheet error: '" << tilesheet_path << "' does not exist." << std::endl;
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

Map::~Map() {
    this->clear();
}

#define CORRUPTED_EXIT \
{\
    std::cerr << "map.cpp (" << __LINE__ << "): '" << map_path << "' corrupted." << std::endl;\
    exit(1);\
}

void Map::make_empty(int tile_width, int tile_height, int cols, int rows) {
    this->clear();
    
    size_t nbytes = sizeof(std::vector<Tile> *) * cols * rows;
    this->bg_tiles = (std::vector<Tile> **)malloc(nbytes);
    this->fg_tiles = (std::vector<Tile> **)malloc(nbytes);
    this->collision = (int *)malloc(sizeof(int) * cols * rows);

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

    this->bg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cols * tile_width, rows * tile_height);
    this->fg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cols * tile_width, rows * tile_height);

    if (this->bg == nullptr || this->fg == nullptr) {
        std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    SDL_SetRenderTarget(renderer, this->bg);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, this->fg);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, game->screen);
}

void Map::read(const char *map_path) {
    FILE *file = fopen(map_path, "rb");
    if (file == NULL) {
        std::cerr << "map.cpp (" << __LINE__ << "): '" << map_path << "' does not exist." << std::endl;
        exit(1);
    }

    uint8_t buffer[1024];
    size_t nbytes;

    // first four bytes of file are tile width, height, columns, and rows 
    nbytes = fread(buffer, 1, 4, file);
    if (nbytes < 4) CORRUPTED_EXIT

    this->tile_width = int(buffer[0]);
    this->tile_height = int(buffer[1]);
    this->cols = int(buffer[2]);
    this->rows = int(buffer[3]);
    // arbitrary maximum tile size
    if (this->tile_width > 64 || this->tile_height > 64) CORRUPTED_EXIT

    // allocate the necessary memory
    this->make_empty(this->tile_width, this->tile_height, this->cols, this->rows);

    // read tilesheets
    while (true) {
        nbytes = fread(buffer, 1, 1, file);
        if (nbytes < 1) CORRUPTED_EXIT

        // buffer[0] is length of tilesheet path; end of tilesheets is a zero
        if (buffer[0] == 0) break;

        char tilesheet_path[256];
        nbytes = fread(tilesheet_path, 1, size_t(buffer[0]), file);
        if (nbytes < buffer[0]) CORRUPTED_EXIT

        tilesheet_path[nbytes] = '\0';
        Tilesheet *tilesheet = new Tilesheet(tilesheet_path, this->tile_width, this->tile_height);
        this->tilesheets.push_back(tilesheet);
        if (this->tilesheets[this->tilesheets.size() - 1]->texture == nullptr) CORRUPTED_EXIT
    }

    // read tiles
    while (true) {
        nbytes = fread(buffer, 1, 6, file);

        if (buffer[0] == 255) {
            // buffer[1:5] aren't for tiles in this case
            if (fseek(file, -5, SEEK_CUR) != 0) CORRUPTED_EXIT
            break;
        } else if (nbytes < 6) CORRUPTED_EXIT

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
            tile.y >= this->tilesheets[tile.tilesheet]->rows
        ) CORRUPTED_EXIT

        // background or foreground tile
        if (buffer[5] == 'b') {
            std::vector<Tile> *vec = this->bg_tiles[coord];
            if (vec == nullptr) {
                vec = new std::vector<Tile>();
                this->bg_tiles[coord] = vec;
            }
            vec->push_back(tile);
        } else if (buffer[5] == 'f') {
            std::vector<Tile> *vec = this->fg_tiles[coord];
            if (vec == nullptr) {
                vec = new std::vector<Tile>();
                this->fg_tiles[coord] = vec;
            }
            vec->push_back(tile);
        } else CORRUPTED_EXIT
    }

    // read objects
    char line[1024];
    while (fgets(line, sizeof(line), file) != nullptr) {
        char obj_id[256], options[768];
        sscanf(line, "%s %[^\n]", obj_id, options);
        this->objects.push_back(std::pair<std::string, std::string>(
            std::string(obj_id), std::string(options)
        ));
    }

    // done reading file
    fclose(file);

    // render all tiles read from the file
    for (int i = 0; i < this->cols * this->rows; i++) {
        render_tile(i % this->cols, i / this->cols);
    }
}

static inline void write_tile(uint8_t *buffer, Tile tile, int x, int y, char ground, FILE *file) {
    buffer[0] = uint8_t(tile.tilesheet);
    buffer[1] = uint8_t(tile.x);
    buffer[2] = uint8_t(tile.y);
    buffer[3] = uint8_t(x);
    buffer[4] = uint8_t(y);
    buffer[5] = uint8_t(ground);
    fwrite(buffer, 1, 6, file);
}

void Map::write(const char *map_path) {
    FILE *file = fopen(map_path, "wb");
    if (file == nullptr) CORRUPTED_EXIT;
    uint8_t buffer[1024];

    // write map width and tile info
    buffer[0] = uint8_t(this->tile_width);
    buffer[1] = uint8_t(this->tile_height);
    buffer[2] = uint8_t(this->cols);
    buffer[3] = uint8_t(this->rows);
    fwrite(buffer, 1, 4, file);

    // write tilesheet info
    for (auto &tilesheet : this->tilesheets) {
        buffer[0] = uint8_t(tilesheet->path.size());
        memcpy((void *)(buffer+1), (void *)tilesheet->path.c_str(), tilesheet->path.size());
        fwrite(buffer, 1, tilesheet->path.size() + 1, file);
    }

    // write a zero signifying end of tilesheets
    buffer[0] = 0;
    fwrite(buffer, 1, 1, file);

    // write tiles
    for (int i = 0; i < this->cols * this->rows; i++) {
        int x = i % this->cols, y = i / this->cols;
        // colliders
        int c = this->collision[i];
        if (c >= 0) {
            buffer[0] = uint8_t(c);
            buffer[3] = uint8_t(x);
            buffer[4] = uint8_t(y);
            buffer[5] = uint8_t('c');
            fwrite(buffer, 1, 6, file);
        }

        // background tiles
        std::vector<Tile> *vec = this->bg_tiles[i];
        if (vec != nullptr)
            for (auto tile : *vec)
                write_tile(buffer, tile, x, y, 'b', file);

        // foreground tiles
        vec = this->fg_tiles[i];
        if (vec != nullptr)
            for (auto tile : *vec)
                write_tile(buffer, tile, x, y, 'f', file);
    }

    // write a 255 signifying end of tiles
    buffer[0] = 255;
    fwrite(buffer, 1, 1, file);

    // write objects
    for (auto obj : this->objects) {
        size_t nbytes = snprintf((char *)buffer, sizeof(buffer), "%s %s\n", obj.first.c_str(), obj.second.c_str());
        fwrite(buffer, 1, nbytes, file);
    }

    fclose(file);
}

void Map::clear() {
    this->tilesheets.clear();
    if (this->bg_tiles != nullptr) {
        free(this->bg_tiles);
        this->bg_tiles = nullptr;
    }
    if (this->fg_tiles != nullptr) {
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &dst_rect); // fill a black square

    // render bg tiles
    std::vector<Tile> *vec = this->bg_tiles[(y * this->cols) + x];
    if (vec != nullptr) {
        SDL_FRect src_rect;
        src_rect.w = float(this->tile_width);
        src_rect.h = float(this->tile_height);

        // render all tiles in order
        for (auto tile : *vec) {
            src_rect.x = float(this->tile_width * tile.x);
            src_rect.y = float(this->tile_height * tile.y);
            SDL_RenderTexture(renderer, this->tilesheets[tile.tilesheet]->texture, &src_rect, &dst_rect);
        }
    }

    SDL_SetRenderTarget(renderer, this->fg);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(renderer, &dst_rect); // fill a transparent square

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
            SDL_RenderTexture(renderer, this->tilesheets[tile.tilesheet]->texture, &src_rect, &dst_rect);
        }
    }
}
