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

    this->off_x = SDL_randf() * 16.0f - 8.0f;
    this->off_y = SDL_randf() * 16.0f - 8.0f;
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
        Mouse *new_mouse = closest_mouse(this->x, this->y, this->stalking_distance, false);

        if (new_mouse == nullptr) {
            // no more stalking if neither mouse is close enough
            this->target_mouse = nullptr;
            this->current_distance = this->stalking_distance;
        } else if (this->target_mouse == nullptr) {
            // no target mouse presently; target the new mouse
            Uint64 target_ticks_offset = SDL_rand(10) * 1000;
            this->target_ticks = game->ticks - target_ticks_offset;
            this->target_mouse = new_mouse;
            this->current_distance = distance_between_points(this->x, this->y, new_mouse->x, new_mouse->y);
        } else {
            this->current_distance = distance_between_points(this->x, this->y, this->target_mouse->x, this->target_mouse->y);

            if (new_mouse != this->target_mouse
                && (this->target_mouse->is_down
                || this->current_distance > this->stalking_distance
                || game->ticks - this->target_ticks > 10000)) {
                this->target_ticks = game->ticks;
                this->target_mouse = new_mouse;
                this->current_distance = distance_between_points(this->x, this->y, new_mouse->x, new_mouse->y);
            }
        }

        // perform action and stay on this tile if close enough and not running away
        if (this->current_distance < this->action_distance && this->tile_choice != TileChoice::SPAZZ) {
            this->action(this->target_mouse);
            this->state = Foe::State::ACTION;
            break;
        }

        // will be walking if not action'ing
        this->state = Foe::State::WALKING;

        int current_x = this->target_x;
        int current_y = this->target_y;

        // move towards closest mouse
        if (this->target_mouse != nullptr) {
            bool path_exists = false;
            switch (this->tile_choice) {
            case Foe::TileChoice::TOWARDS:
                path_exists = foe_move_towards(this->target_mouse, &this->target_x, &this->target_y, this->strafe);
                break;
            case Foe::TileChoice::AWAY:
                path_exists = foe_move_away(this->target_mouse, &this->target_x, &this->target_y);
                break;
            case Foe::TileChoice::CIRCLE:
                path_exists = foe_move_circle(this->target_mouse, &this->target_x, &this->target_y);
                break;
            case Foe::TileChoice::SPAZZ:
                if (SDL_rand(2)) {
                    path_exists = foe_move_circle(this->target_mouse, &this->target_x, &this->target_y);
                } else {
                    path_exists = false; // random tile
                }
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

        if (game->ticks - this->direction_timer > 100 && this->_displayed_direction != this->direction) {
            this->direction_timer = game->ticks;

            int diff_up = (this->direction < this->_displayed_direction ? this->direction + 8 : this->direction) - this->_displayed_direction;
            int diff_down = this->_displayed_direction - (this->direction > this->_displayed_direction ? this->direction - 8 : this->direction);

            if (diff_up <= diff_down) {
                this->_displayed_direction++;
            } else {
                // diff_down < diff_up
                this->_displayed_direction--;
            }

            if (this->_displayed_direction < 0)
                this->_displayed_direction = 7;
            else if (this->_displayed_direction > 7)
                this->_displayed_direction = 0;
        }
    } break;

    case HURT: {
        // for when out of the hurt
        this->start_ticks = game->ticks - this->walk_offset;
    } break;

    default: break;
    }
}
