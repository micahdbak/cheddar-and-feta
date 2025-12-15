#include "freedom.h"
#include "game.h"
#include "mouse.h"
#include "save_data.h"

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
        if (game->ticks - this->timer_start > 2000) {
            game->map = "maps/credits";
        }

        return;
    }

    float cdist = distance_between_points(this->x, this->y, cheddar->x, cheddar->y);
    float fdist = distance_between_points(this->x, this->y, feta->x, feta->y);

    // when either mouse goes 64px away from the ladder
    if (cdist > 64.0f || fdist > 64.0f) {
        // eternally dance
        cheddar->force_dance(UINT64_MAX);
        feta->force_dance(UINT64_MAX);
        this->timer_start = game->ticks;

        save.puti(GAME_DONE, 1);
        save.write_file(save.geti(SAVE_FILE));
    }
}