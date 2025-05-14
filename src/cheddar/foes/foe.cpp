#include "foe.h"
#include "game.h"
#include "mouse.h"
#include "spawner.h"

#include <iostream>

std::vector<Foe *> foes;

#define CENTER_TILE_X(_x) ((float)(int)((_x * game->tile_width) + (game->tile_width / 2)) + 0.5f)
#define CENTER_TILE_Y(_y) ((float)(int)((_y * game->tile_height) + (game->tile_height / 2)) + 0.5f)

Foe::Foe(float x, float y, int spawner_id, int speed, float stalking_distance, float action_distance):
    spawner_id(spawner_id), speed(speed), stalking_distance(stalking_distance), action_distance(action_distance) {
    this->target_x = (int)x / game->tile_width;
    this->target_y = (int)y / game->tile_height;

    // ensure starting x/y is in the center of the corresponding tile
    this->x = CENTER_TILE_X(this->target_x);
    this->y = CENTER_TILE_Y(this->target_y);

    this->state = Foe::State::IDLE;

    foes.push_back(this);

    this->speed += (SDL_randf() * 4.0f) - 2.0f;
}

Foe::~Foe() {
    this->remove_from_foes();

    if (spawners[this->spawner_id] != nullptr) {
        spawners[this->spawner_id]->log_death();
    }
}

void Foe::remove_from_foes() {
    // remove this foe from the foes list
    for (auto it = foes.begin(); it != foes.end(); it++) {
        if (*it == this) {
            foes.erase(it);
            break;
        }
    }
}

void Foe::foe_step() {
    switch (this->state) {
    case Foe::State::IDLE: {
        // find target mouse by distance
        float cheddar_dist = distance_between_points(cheddar->x, cheddar->y, this->x, this->y);
        float feta_dist = distance_between_points(feta->x, feta->y, this->x, this->y);
        float target_dist = cheddar_dist < feta_dist ? cheddar_dist : feta_dist;
        Mouse *target_mouse = cheddar_dist < feta_dist ? cheddar : feta;

        // perform action and stay on this tile if close enough
        if (target_dist < this->action_distance) {
            this->action(target_mouse);
            this->state = Foe::State::ACTION;
            break;
        }

        // will be walking if not action'ing
        this->state = Foe::State::WALKING;

        int current_x = this->target_x;
        int current_y = this->target_y;

        // move towards closest mouse
        if (target_dist < this->stalking_distance) {
            bool path_exists = foe_move_towards(target_mouse, &this->target_x, &this->target_y);

            // we are on top of the mouse - pick a random tile
            if (!path_exists) {
                foe_pick_random(&this->target_x, &this->target_y);
            }
        } else {
            // no mouse close enough; pick a random tile and move there
            foe_pick_random(&this->target_x, &this->target_y);
        }

        this->x_dir = this->target_x - current_x;
        this->y_dir = this->target_y - current_y;
        this->direction = direction_from_dirs(x_dir, y_dir);

        if (this->x_dir == 0 && this->y_dir == 0) {
            this->state = Foe::State::IDLE;
            std::cout << "possible block?" << std::endl;
        }
    } break;

    case Foe::State::WALKING: {
        float dx = float(this->x_dir) * (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * this->speed * game->delta;
        float dy = float(this->y_dir) * (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * this->speed * game->delta;

        // move maximum 1 pixel per frame
        if (fabs(dx) > 1.0f) {
            dx /= fabs(dx);
        }

        // move maximum 1 pixel per frame
        if (fabs(dy) > 1.0f) {
            dy /= fabs(dy);
        }

        float new_x = this->x + dx;
        float new_y = this->y + dy;

        float target_x_f = CENTER_TILE_X(this->target_x);
        float target_y_f = CENTER_TILE_Y(this->target_y);

        float dist1 = fabs(target_x_f - this->x) + fabs(target_y_f - this->y);
        float dist2 = fabs(target_x_f - new_x) + fabs(target_y_f - new_y);

        // check for overshoot (no missing the tile) + within 8 pixels (no teleporting a full tile)
        if (dist2 > dist1 && dist2 < 8.0f) {
            // stand on the center of this tile
            this->x = target_x_f;
            this->y = target_y_f;
            this->state = Foe::State::IDLE; // next step will select a new tile
        } else {
            this->x = new_x;
            this->y = new_y;
        }
    } break;

    // ACTION, HURT, DEAD must be terminated by the parent
    default: break;
    }
}
