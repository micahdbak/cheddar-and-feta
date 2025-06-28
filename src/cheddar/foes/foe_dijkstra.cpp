#include "SDL3/SDL_stdinc.h"
#include "bmp_texture.h"
#include "foe.h"
#include "game.h"
#include "mouse.h"

#include <iostream>
#include <queue>

#define FREE_IF(a)      if ((a)) { free(a); }
#define COORD(x, y)     (((y) * cols) + (x))
#define DIST_ADJACENT   10000
#define DIST_DIAGONAL   14142
#define MAX_DISTANCE    DIST_ADJACENT * 16 // 16 tiles radius
#define COLLIDER        -1
#define TARGET          -2

struct _QItem {
    std::pair<int, int> coord;
    int dist;

    bool operator<(const _QItem &other) const {
        return this->dist < other.dist;
    }
};

struct _Neighbour {
    std::pair<int, int> coord;
    bool is_diagonal;
};

static int rows = 0, cols = 0;
static int *_cheddar_dist = nullptr, *_feta_dist = nullptr;
static int *_cheddar_prev = nullptr, *_feta_prev = nullptr;

static void _init_matrices() {
    FREE_IF(_cheddar_dist);
    FREE_IF(_feta_dist);
    FREE_IF(_cheddar_prev);
    FREE_IF(_feta_prev);

    rows = game->rows;
    cols = game->cols;

    _cheddar_dist = (int *)malloc((rows * cols) * sizeof(int));
    _feta_dist = (int *)malloc((rows * cols) * sizeof(int));
    _cheddar_prev = (int *)malloc((rows * cols) * sizeof(int));
    _feta_prev = (int *)malloc((rows * cols) * sizeof(int));
}

static void _neighbours(int x, int y, std::vector<_Neighbour> &neighbours) {
    for (int _y = y-1; _y <= y+1; _y++) {
        for (int _x = x-1; _x <= x+1; _x++) {
            if (_x < 0 || _y < 0 || _x >= cols || _y >= rows || // out of bounds
                game->collision[COORD(_x, _y)] >= 0 || // collider present
                (_x == x && _y == y)) { // source tile
                continue;
            }

            // valid neighbour
            neighbours.push_back(_Neighbour{
                std::pair<int, int>(_x, _y),
                (_x != x && _y != y) // neighbour is diagonal if neither x and y are in common
            });
        }
    }
}

void foe_path_find(Mouse *mouse) {
    if (game->collision == nullptr) {
        std::cerr << "foe_path_find error: called without game->collision" << std::endl;
        std::exit(1);
    }

    if (game->rows != rows) {
        _init_matrices();
    }

    int *dist = mouse->is_feta ? _feta_dist : _cheddar_dist;
    int *prev = mouse->is_feta ? _feta_prev : _cheddar_prev;

    std::priority_queue<_QItem> queue;
    std::vector<bool> visited(rows * cols, false);
    std::vector<_Neighbour> neighbours;

    // starting coordinates
    int mouse_tile_x = (int)mouse->x / game->tile_width;
    int mouse_tile_y = (int)mouse->y / game->tile_height;
    std::pair<int, int> start(mouse_tile_x, mouse_tile_y);
    queue.push(_QItem{ start, 0 });

    // prepare the distance and marching matrices
    for (int i = 0; i < rows * cols; i++) {
        dist[i] = MAX_DISTANCE;
        prev[i] = COLLIDER;
    }

    prev[COORD(mouse_tile_x, mouse_tile_y)] = TARGET;
    dist[COORD(start.first, start.second)] = 0;

    while (!queue.empty()) {
        _QItem u = queue.top();
        queue.pop();
        int u_i = COORD(u.coord.first, u.coord.second);

        // don't re-visit tiles
        if (visited[u_i]) continue;
        //visited[u_i] = true;

        neighbours.clear();
        _neighbours(u.coord.first, u.coord.second, neighbours);

        // iterate over neighbours of u
        for (_Neighbour v : neighbours) {
            int v_i = COORD(v.coord.first, v.coord.second);

            // calculate distance to v from u and distance from v to start
            int v_dist = v.is_diagonal ? DIST_DIAGONAL : DIST_ADJACENT;
            int alt = dist[u_i] + v_dist;

            if (alt < dist[v_i] && alt < MAX_DISTANCE) {
                prev[v_i] = u_i;
                dist[v_i] = alt;
                queue.push(_QItem{v.coord, alt});
            }
        }
    }

    // dist and prev are populated; all done
}

bool foe_move_towards(Mouse *mouse, int *x, int *y, FoeStrafe strafe) {
    int *prev = mouse->is_feta ? _feta_prev : _cheddar_prev;

    if (prev == nullptr) {
        return false;
    }

    int _x = cnf_clamp(*x, 0, cols-1);
    int _y = cnf_clamp(*y, 0, rows-1);

    int i = prev[COORD(_x, _y)];

    if (i < 0 || i >= rows * cols) {
        return false;
    }

    *x = i % cols;
    *y = i / rows;

    // 25% chance strafe left, 25% chance strafe right, 50% don't strafe
    if (strafe == FoeStrafe::NO_STRAFE) {
        switch (SDL_rand(4)) {
        case 0: strafe = FoeStrafe::STRAFE_LEFT; break;
        case 1: strafe = FoeStrafe::STRAFE_RIGHT; break;
        default: break;
        }
    }

    // 50% chance to not strafe
    if (strafe != FoeStrafe::NO_STRAFE && SDL_rand(2) == 1) {
        int x_dir = *x - _x;
        int y_dir = *y - _y;
        int direction = direction_from_dirs(x_dir, y_dir);

        // alter direction according to strafe
        if (strafe == FoeStrafe::STRAFE_LEFT) direction -= 1;
        if (strafe == FoeStrafe::STRAFE_RIGHT) direction += 1;

        // cycle
        if (direction < 0) direction = 7;
        if (direction > 7) direction = 0;

        // get new x/y dir
        dirs_from_direction(direction, &x_dir, &y_dir);

        int strafe_x = _x + x_dir;
        int strafe_y = _y + y_dir;
        int strafe_coord = COORD(strafe_x, strafe_y);

        // strafe goes to a free tile
        if (strafe_coord >= 0 && strafe_coord < rows * cols && game->collision[strafe_coord] < 0) {
            *x = strafe_coord % cols;
            *y = strafe_coord / rows;
        } // else, leave x/y untouched
    }

    return true;
}

void foe_pick_random(int *x, int *y) {
    std::vector<_Neighbour> neighbours;
    _neighbours(*x, *y, neighbours);

    if (!neighbours.empty()) {
        int i = SDL_rand(neighbours.size());
        *x = neighbours[i].coord.first;
        *y = neighbours[i].coord.second;
    }
}

static SDL_FRect debug_src_rect[] = {
    SDL_FRect{ 0.0f, 0.0f, 16.0f, 16.0f },
    SDL_FRect{ 16.0f, 0.0f, 16.0f, 16.0f },
    SDL_FRect{ 32.0f, 0.0f, 16.0f, 16.0f },
    SDL_FRect{ 48.0f, 0.0f, 16.0f, 16.0f },
    SDL_FRect{ 64.0f, 0.0f, 16.0f, 16.0f },
    SDL_FRect{ 80.0f, 0.0f, 16.0f, 16.0f },
    SDL_FRect{ 96.0f, 0.0f, 16.0f, 16.0f },
    SDL_FRect{ 112.0f, 0.0f, 16.0f, 16.0f },
    SDL_FRect{ 128.0f, 0.0f, 16.0f, 16.0f }
};

SDL_FRect *debug_dst_rect = nullptr;
SDL_Texture *debug_tiles_texture = nullptr;

void foe_debug_tile(int x, int y) {
    if (debug_dst_rect == nullptr) {
        debug_dst_rect = (SDL_FRect *)malloc(sizeof(SDL_FRect) * rows * cols);
        debug_tiles_texture = load_bmp_texture("sprites/foedebug.bmp");
    }

    float center_x = CENTER_TILE_X(x);
    float center_y = CENTER_TILE_Y(y);

    float dist1 = distance_between_points(center_x, center_y, cheddar->x, cheddar->y);
    float dist2 = distance_between_points(center_x, center_y, feta->x, feta->y);
    Mouse *mouse = nullptr;

    if (dist1 < dist2) {
        mouse = cheddar;
    } else {
        mouse = feta;
    }

    int *prev = mouse->is_feta ? _feta_prev : _cheddar_prev;

    if (x < 0 || x >= cols || y < 0 || y >= rows) {
        return;
    }

    SDL_FRect *src_rect, *dst_rect;

    int prev_tile = prev[COORD(x, y)];
    if (prev_tile >= 0) {
        int px = prev_tile % cols;
        int py = prev_tile / rows;

        int x_dir = px - x;
        int y_dir = py - y;
        int direction = direction_from_dirs(x_dir, y_dir);

        src_rect = &debug_src_rect[direction];
    } else {
        src_rect = &debug_src_rect[8];
    }

    debug_dst_rect[COORD(x, y)] = SDL_FRect{
        float((x * game->tile_width) - game->corner_x),
        float((y * game->tile_height) - game->corner_y),
        16.0f, 16.0f
    };
    dst_rect = &debug_dst_rect[COORD(x, y)];

    game->push_sprite("sprites/foedebug.bmp", debug_tiles_texture, src_rect, dst_rect, 0);
}
