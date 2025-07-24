#include "foe.h"
#include "game.h"
#include "mouse.h"
#include "spawner.h"

#include <iostream>

std::vector<Foe *> foes;

static std::vector<int> speed_offsets = { 0, 4, 8, 4, 2, -2, -4, -8, -4 };
static int speed_offsets_i = 0;

Foe::Foe(float x, float y, int spawner_id, int speed, float stalking_distance, float action_distance):
    spawner_id(spawner_id), speed(speed), stalking_distance(stalking_distance), action_distance(action_distance) {
    this->target_x = (int)x / game->tile_width;
    this->target_y = (int)y / game->tile_height;

    // ensure starting x/y is in the center of the corresponding tile
    this->x = CENTER_TILE_X(this->target_x);
    this->y = CENTER_TILE_Y(this->target_y);

    this->state = Foe::State::IDLE;

    foes.push_back(this);

    this->speed += speed_offsets[speed_offsets_i];
    speed_offsets_i++;

    if (speed_offsets_i >= speed_offsets.size())
        speed_offsets_i = 0;

    switch (speed_offsets_i % 3) {
    case 1: this->strafe = FoeStrafe::STRAFE_LEFT; break;
    case 2: this->strafe = FoeStrafe::STRAFE_RIGHT; break;
    default: this->strafe = FoeStrafe::NO_STRAFE; break;
    }
}

Foe::~Foe() {
    this->remove_from_foes();

    if (this->spawner_id >= 0 && spawners[this->spawner_id] != nullptr) {
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

void Foe::attack(int damage) {
    this->attack_internal(damage);
    this->walk_offset = game->ticks - this->start_ticks;
}

void Foe::foe_step() {
    switch (this->state) {
    case Foe::State::IDLE: {
        // find target mouse by distance
        float cheddar_dist = distance_between_points(cheddar->x, cheddar->y, this->x, this->y);
        float feta_dist = distance_between_points(feta->x, feta->y, this->x, this->y);
        float target_dist = cheddar_dist < feta_dist ? cheddar_dist : feta_dist;
        this->current_distance = target_dist;
        Mouse *target_mouse = cheddar_dist < feta_dist ? cheddar : feta;

        // perform action and stay on this tile if close enough and not running away
        if (target_dist < this->action_distance && this->tile_choice != Foe::TileChoice::AWAY) {
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
            bool path_exists = false;
            switch (this->tile_choice) {
            case Foe::TileChoice::TOWARDS:
                path_exists = foe_move_towards(target_mouse, &this->target_x, &this->target_y, this->strafe);
                break;
            case Foe::TileChoice::AWAY:
                path_exists = foe_move_away(target_mouse, &this->target_x, &this->target_y);
                break;
            case Foe::TileChoice::CIRCLE:
                path_exists = foe_move_circle(target_mouse, &this->target_x, &this->target_y);
                break;
            }

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
        }

        this->start_x = CENTER_TILE_X(current_x);
        this->start_y = CENTER_TILE_Y(current_y);
        this->start_ticks = game->ticks;

        bool diagonal = this->x_dir != 0 && this->y_dir != 0;
        this->walking_time = diagonal ? (int)((float)this->speed * DIAG_MULTIPLIER2) : this->speed;
    } break;

    case Foe::State::FORCE_RANDOM_TILE: {
        this->state = Foe::State::WALKING;

        int current_x = this->target_x;
        int current_y = this->target_y;

        foe_pick_random(&this->target_x, &this->target_y);

        this->x_dir = this->target_x - current_x;
        this->y_dir = this->target_y - current_y;
        this->direction = direction_from_dirs(x_dir, y_dir);

        if (this->x_dir == 0 && this->y_dir == 0) {
            this->state = Foe::State::IDLE;
        }

        this->start_x = CENTER_TILE_X(current_x);
        this->start_y = CENTER_TILE_Y(current_y);
        this->start_ticks = game->ticks;

        bool diagonal = this->x_dir != 0 && this->y_dir != 0;
        this->walking_time = diagonal ? (int)((float)this->speed * DIAG_MULTIPLIER2) : this->speed;
    } break;

    case Foe::State::WALKING: {
        int elapsed = game->ticks - this->start_ticks;
        float perc = (float)elapsed / (float)this->walking_time;

        float dx = (float)this->x_dir * perc * 16.0f;
        float dy = (float)this->y_dir * perc * 16.0f;

        this->x = this->start_x + dx;
        this->y = this->start_y + dy;

        if (elapsed > this->walking_time) {
            this->x = CENTER_TILE_X(this->target_x);
            this->y = CENTER_TILE_Y(this->target_y);
            this->state = Foe::State::IDLE; // next step will select a new tile
        }
    } break;

    case HURT: {
        // for when out of the hurt
        this->start_ticks = game->ticks - this->walk_offset;
    } break;

    default: break;
    }
}
