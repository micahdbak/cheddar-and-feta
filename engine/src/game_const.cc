#include "game.h"

int direction_from_dirs(int x_dir, int y_dir) {
  if (x_dir < 0) {
    return 6 + y_dir;
  } else if (x_dir > 0) {
    return 2 - y_dir;
  } else if (y_dir < 0) {
    return 4;
  } else {
    return 0;
  }
}

void dirs_from_direction(int direction, int* x_dir, int* y_dir) {
  switch (direction) {
    case 0:
      *x_dir = 0, *y_dir = 1;
      break;
    case 1:
      *x_dir = 1, *y_dir = 1;
      break;
    case 2:
      *x_dir = 1, *y_dir = 0;
      break;
    case 3:
      *x_dir = 1, *y_dir = -1;
      break;
    case 4:
      *x_dir = 0, *y_dir = -1;
      break;
    case 5:
      *x_dir = -1, *y_dir = -1;
      break;
    case 6:
      *x_dir = -1, *y_dir = 0;
      break;
    case 7:
      *x_dir = -1, *y_dir = 1;
      break;
    default:
      break;
  }
}

void dir_to_point(float x1, float y1, float x2, float y2, int* x_dir,
                  int* y_dir) {
  float angle = atan2(y2 - y1, x2 - x1);  // [-M_PI,M_PI]
  angle += M_PI;                          // [0,2.0*M_PI]
  angle /= M_PI;                          // [0,2.0]
  angle *= 4.0f;                          // [0,8.0]
  angle += 0.5f;                          // [0.5,8.5] - for integer rounding

  switch (int(angle)) {
    case 0:
      *x_dir = -1, *y_dir = 0;
      break;
    case 1:
      *x_dir = -1, *y_dir = -1;
      break;
    case 2:
      *x_dir = 0, *y_dir = -1;
      break;
    case 3:
      *x_dir = 1, *y_dir = -1;
      break;
    case 4:
      *x_dir = 1, *y_dir = 0;
      break;
    case 5:
      *x_dir = 1, *y_dir = 1;
      break;
    case 6:
      *x_dir = 0, *y_dir = 1;
      break;
    case 7:
      *x_dir = -1, *y_dir = 1;
      break;
    case 8:
      *x_dir = -1, *y_dir = 0;
      break;
  }
}

void Game::make_map_rect(int x, int y, int w, int h, SDL_FRect* src_rect,
                         SDL_FRect* dst_rect) const {
  if (x < 0) {
    dst_rect->x = float(-1 * x);
    dst_rect->w = float(cnf_min(SCREEN_WIDTH, w));
    src_rect->x = 0.0f;
  } else {
    dst_rect->x = 0.0f;
    dst_rect->w = float(cnf_min(SCREEN_WIDTH, w - x));
    src_rect->x = float(x);
  }

  if (y < 0) {
    dst_rect->y = float(-1 * y);
    dst_rect->h = float(cnf_min(SCREEN_HEIGHT, h));
    src_rect->y = 0.0f;
  } else {
    dst_rect->y = 0.0f;
    dst_rect->h = float(cnf_min(SCREEN_HEIGHT, h - y));
    src_rect->y = float(y);
  }

  src_rect->w = dst_rect->w;
  src_rect->h = dst_rect->h;
}

// below `_sign` and `_point_in_triangle` functions from:
// https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle

static float _sign(SDL_FPoint p1, SDL_FPoint p2, SDL_FPoint p3) {
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

static bool _point_in_triangle(SDL_FPoint pt, SDL_FPoint v1, SDL_FPoint v2,
                               SDL_FPoint v3) {
  float d1, d2, d3;
  bool has_neg, has_pos;

  d1 = _sign(pt, v1, v2);
  d2 = _sign(pt, v2, v3);
  d3 = _sign(pt, v3, v1);

  has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

  return !(has_neg && has_pos);
}

bool Game::point_in_collider(float x, float y) const {
  // no collision
  if (this->collision == nullptr) return false;

  // flooring is intentional
  int _x = int(x) / this->tile_width;
  int _y = int(y) / this->tile_height;

  // out of map is automatic collision
  if (x < 0.0f || y < 0.0f || _x >= this->cols || _y >= this->rows) return true;

  int coord = (_y * this->cols) + _x;

  // no collider at point or invalid collider
  if (this->collision[coord] < 0 || this->collision[coord] >= n_MapColliders)
    return false;

  Quad quad = this->colliders[this->collision[coord]];
  SDL_FPoint point;
  point.x = x - float(_x * this->tile_width);
  point.y = y - float(_y * this->tile_width);

  // return true if the point lies in either triangles making up the collider's
  // quad
  return _point_in_triangle(point, quad.vertex[0], quad.vertex[1],
                            quad.vertex[2]) ||
         _point_in_triangle(point, quad.vertex[2], quad.vertex[3],
                            quad.vertex[0]);
}

bool Game::in_sight(int x0, int y0, int x1, int y1, int* next_x,
                    int* next_y) const {
  x0 = x0 / this->tile_width;
  y0 = y0 / this->tile_width;
  x1 = x1 / this->tile_width;
  y1 = y1 / this->tile_height;

  int dx = abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int error = dx + dy;
  bool next_set = false;

  while (true) {
    // return false if there is a collider in the way
    if (this->collision[(y0 * this->cols) + x0] >= 0) return false;

    int e2 = 2 * error;
    if (e2 >= dy) {
      if (x0 == x1) break;
      error = error + dy;
      x0 = x0 + sx;
    }
    if (e2 <= dx) {
      if (y0 == y1) break;
      error = error + dx;
      y0 = y0 + sy;
    }

    if (!next_set && next_x != NULL && next_y != NULL) {
      *next_x = x0 * this->tile_width;
      *next_y = y0 * this->tile_height;
      next_set = true;
    }
  }

  if (!next_set && next_x != NULL && next_y != NULL) {
    *next_x = x0 * this->tile_width;
    *next_y = y0 * this->tile_height;
    next_set = true;
  }

  // no colliders in the way; target is in sight
  return true;
}

void Game::random_target(int x, int y, int* next_x, int* next_y) const {
  if (next_x == NULL || next_y == NULL) return;  // legit wtf if this happens

  x /= this->tile_width;
  y /= this->tile_height;

  std::vector<std::pair<int, int>> candidates;

  int start_x = cnf_clamp(x - 1, 0, this->cols - 1);
  int start_y = cnf_clamp(y - 1, 0, this->rows - 1);
  int end_x = cnf_clamp(x + 1, 0, this->cols - 1);
  int end_y = cnf_clamp(y + 1, 0, this->rows - 1);

  for (int _x = start_x; _x <= end_x; _x++) {
    for (int _y = start_y; _y <= end_y; _y++) {
      if (_x == x && _y == y) continue;

      if (this->collision[(_y * this->cols) + _x] < 0) {
        candidates.push_back({_x, _y});
      }
    }
  }

  std::pair<int, int> target = candidates[SDL_rand(candidates.size())];
  *next_x = target.first * this->tile_width;
  *next_y = target.second * this->tile_height;
}

static char nullbyte = '\0';

const char* next_line(const char* arr) {
  size_t i = 0;

  while (*arr != '\n' && *arr != '\0' && i < 1024) {
    arr++;
    i++;
  }

  if (i >= 1024) {
    return &nullbyte;
  }

  if (*arr == '\n') {
    arr++;
  }

  return arr;
}