#include "freedom.h"

#include "game.h"
#include "mouse.h"
#include "save_data.h"
#include "utils.h"

Freedom::Freedom(int x, int y) {
  this->x = x;
  this->y = y;
}

Freedom::~Freedom() {}

void Freedom::step() {
  if (cheddar == nullptr || feta == nullptr) {
    return;
  }

  if (this->timer_start > 0) {
    if (thoom::game->ticks - this->timer_start > 2000) {
      thoom::game->map = "maps/credits";
    }

    return;
  }

  float cdist =
      THOOM_DISTANCE_BETWEEN_POINTS(this->x, this->y, cheddar->x, cheddar->y);
  float fdist =
      THOOM_DISTANCE_BETWEEN_POINTS(this->x, this->y, feta->x, feta->y);

  // when either mouse goes 64px away from the ladder
  if (cdist > 64.0f || fdist > 64.0f) {
    // eternally dance
    cheddar->force_dance(UINT64_MAX);
    feta->force_dance(UINT64_MAX);
    this->timer_start = thoom::game->ticks;

    thoom::save.puti(GAME_DONE, 1);
    thoom::save.write_file(thoom::save.geti(SAVE_FILE));
  }
}