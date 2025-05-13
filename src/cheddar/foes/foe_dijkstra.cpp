#include "SDL3/SDL_stdinc.h"
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

bool foe_move_towards(Mouse *mouse, int *x, int *y) {
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

void debug_about_tile(int x, int y, int *distance, bool *is_cheddar) {
    
}
