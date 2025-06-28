#include "foes/foe.h"
#include "game.h"
#include "items/item.h"
#include "mouse.h"
#include "net_agent.h"
#include "save_data.h"

#include "items/toothpick.h"

#include <iostream>
#include <cmath>

bool mice_locked = false;
bool _locked_due_to_loading = false;
Mouse *cheddar = nullptr, *feta = nullptr;

Mouse::Mouse(int x, int y, bool is_feta) {
    this->is_feta = is_feta;
    if (!this->is_feta) {
        if (cheddar != nullptr) {
            std::cerr << "Mouse::Mouse error: another Cheddar exists..?" << std::endl;
            exit(1);
        }
        cheddar = this;

        // add feta to the map
        char options[256];
        snprintf(options, sizeof(options), "%d,%d,1", x, y);
        game->push_object(MOUSE_OBJ, std::string(options));
        this->sprite = new Sprite("sprites/cheddar.bmp", 32, 32, 250);
        this->name = "Cheddar";
    } else {
        if (feta != nullptr) {
            std::cerr << "Mouse::Mouse error: another Feta exists..?" << std::endl;
            exit(1);
        }
        feta = this;
        this->sprite = new Sprite("sprites/feta.bmp", 32, 32, 250);
        this->name = "Feta";
    }

    this->hat = new Sprite("sprites/hat_cap.bmp", 16, 16, 0);
    this->hat_dst.w = 16.0f;
    this->hat_dst.h = 16.0f;

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;

    this->x = float(x);
    this->y = float(y);
    this->tile_x = x / game->tile_width;
    this->tile_y = y / game->tile_height;
    foe_path_find(this);

    if (save.data[this->is_feta ? "feta_data" : "cheddar_data"] == "true") {
        save.data[this->is_feta ? "feta_data" : "cheddar_data"] = "false";
        this->x = str_to_float(save.data[this->is_feta ? "feta_x" : "cheddar_x"].c_str());
        this->y = str_to_float(save.data[this->is_feta ? "feta_y" : "cheddar_y"].c_str());
        this->sprite->set_animation(atoi(save.data[this->is_feta ? "feta_animation" : "cheddar_animation"].c_str()));
    }

    if (!this->is_feta) {
        game->set_view(this->x, this->y);
    }
}

Mouse::~Mouse() {
    delete this->sprite;
    this->sprite = nullptr;
    delete this->hat;
    this->hat = nullptr;
    if (this->is_feta) {
        cheddar = nullptr;
    } else {
        feta = nullptr;
    }
}

void Mouse::save_data() {
    if (!this->is_feta) {
        save.data["cheddar_data"] = "true";
        char buff[100];
        float_to_str(this->x, buff, sizeof(buff));
        save.data["cheddar_x"] = buff;
        float_to_str(this->y, buff, sizeof(buff));
        save.data["cheddar_y"] = buff;
        snprintf(buff, sizeof(buff), "%d", int(this->sprite->animation));
        save.data["cheddar_animation"] = buff;
    } else {
        save.data["feta_data"] = "true";
        char buff[100];
        float_to_str(this->x, buff, sizeof(buff));
        save.data["feta_x"] = buff;
        float_to_str(this->y, buff, sizeof(buff));
        save.data["feta_y"] = buff;
        snprintf(buff, sizeof(buff), "%d", int(this->sprite->animation));
        save.data["feta_animation"] = buff;
    }
}

void Mouse::post_save_data() {
    save.data[this->is_feta ? "feta_data" : "cheddar_data"] = "false";
}

static bool _is_collision(float x, float y) {
    return game->point_in_collider(x+2.0f, y) ||
        game->point_in_collider(x-2.0f, y) ||
        game->point_in_collider(x, y-2.0f);
}

#define ATTACKING_ANIMATION 8
#define ATTACKED_ANIMATION  16
#define THROWING_ANIMATION  24
#define EATING_ANIMATION    32
#define DANCING_ANIMATION   33
#define SLEEPING_ANIMATION  34

void Mouse::step() {
    if (!_locked_due_to_loading && game->displaying_load_screen) {
        _locked_due_to_loading = true;
        mice_locked = true;
    } else if (_locked_due_to_loading && !game->displaying_load_screen) {
        mice_locked = false;
    }

    if (mice_locked) {
        game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
        return;
    }

    Controller *controller = this->is_feta ? &remote_controller : &local_controller;

    if (!this->items.empty()) {
        this->sel_item += controller->is_hit(R1) - controller->is_hit(L1);
        if (this->sel_item < -1)
            this->sel_item = this->items.size() - 1;
        else if (this->sel_item > this->items.size() - 1)
            this->sel_item = -1;
    } else {
        this->sel_item = -1;
    }

    std::string sel_item_id = this->sel_item < 0 ? ITEM_NONE : this->items[this->sel_item].item_id;
    int sel_item_count = this->sel_item < 0 ? 0 : this->items[this->sel_item].count;
    if (!item_info.contains(sel_item_id)) {
        this->sel_item = -1;
        sel_item_id = ITEM_NONE; // wtf
    }

    if (!this->is_feta)
        game->draw_hud(game->ui, sel_item_id, sel_item_count, this->health, this->max_health, this->cheese);

    // cycle through available foes to find which is closest
    if (!foes.empty()) {
        this->check_foe++;
        if (this->check_foe >= foes.size())
            this->check_foe = 0;

        Foe *foe = foes[this->check_foe];
        if (foe != nullptr) {
            // calculate direction to foe
            int x_dir, y_dir;
            dir_to_point(this->x, this->y, foe->x, foe->y, &x_dir, &y_dir);
            int foe_direction = direction_from_dirs(x_dir, y_dir);

            // current direction that the player is facing
            int facing_direction = this->sprite->animation % 8;

            // if the foe being checked is the previously determined closest foe
            if (foe == this->closest_foe) {
                // if no longer facing this foe
                if (facing_direction != foe_direction) {
                    // then set the closest foe to nullptr and wait to find the next closest foe in this direction
                    this->closest_foe = nullptr;
                    this->closest_distance = FLT_MAX;
                } else {
                    // if still facing it, update its distance, incase it has moved further away
                    this->closest_distance = distance_between_points(foe->x, foe->y, this->x, this->y);
                }
            } else if (facing_direction == foe_direction) {
                // this foe is not the closest foe; check its distance
                float distance = distance_between_points(foe->x, foe->y, this->x, this->y);

                // if it is closer than the closest foe, update it to become the closest foe
                if (this->closest_foe == nullptr || distance < this->closest_distance) {
                    this->closest_foe = foe;
                    this->closest_distance = distance;
                }
            }
        }
    }

    float mov_speed = 0.0f;
    int x_dir = 0, y_dir = 0;

    switch (this->is_busy) {
    case FALSE:
        // sleeping when disconnected
        if (this->is_feta &&
            (game->net_state == NetworkAgent::State::NO_CONNECTION ||
            game->net_state == NetworkAgent::State::WAITING_FOR_PEER)) {
            this->sprite->set_animation(SLEEPING_ANIMATION);
            this->sprite->update_frame();
            this->sprite->interval_ms = 250;

            break; // don't do anything but sleep
        }

        // dancing
        if (controller != nullptr && controller->is_down(R2)) {
            this->sprite->set_animation(DANCING_ANIMATION);
            this->sprite->update_frame();
            this->sprite->interval_ms = 200;

            break; // don't do anything but dance
        } else if (this->sprite->animation == DANCING_ANIMATION) {
            this->sprite->set_animation(0); // facing down
        }

        // move using arrow keys
        if (controller != nullptr) {
            x_dir = int(controller->is_down(RIGHT)) - int(controller->is_down(LEFT));
            y_dir = int(controller->is_down(DOWN)) - int(controller->is_down(UP));
        }

        // update sprite frame and animation only if moving
        if (x_dir != 0 || y_dir != 0) {
            this->sprite->set_animation(direction_from_dirs(x_dir, y_dir));
            this->sprite->update_frame();
        } else {
            // otherwise show only the first frame
            this->sprite->set_frame(0);
        }

        mov_speed = this->max_mov_speed / 2.0f;
        this->sprite->interval_ms = 250;

        // run
        if (controller != nullptr && controller->is_down(L2)) {
            mov_speed = this->max_mov_speed;
            this->sprite->interval_ms = 100;
        }

        // attack / use item
        if (controller != nullptr && controller->is_hit(ACTION1)) {
            this->busy_ticks = game->ticks;

            switch (item_info[sel_item_id].type) {
            case USEFUL:
            case THROWABLE:
                if (x_dir == 0 && y_dir == 0)
                    dirs_from_direction(this->sprite->animation % 8, &x_dir, &y_dir);

                char options[256];
                snprintf(options, sizeof(options), "%d,%d,%d,%d,%d", int(this->x), int(this->y), x_dir, y_dir, this->id);
                game->push_object(sel_item_id + USE_OBJ, std::string(options));

                if (this->remove_item(sel_item_id)) {
                    this->sel_item = -1;
                } else {
                    if (this->sel_item >= this->items.size())
                        this->sel_item--;
                }

                if (item_info[sel_item_id].type == THROWABLE) {
                    this->is_busy = THROWING;

                    // set animation to throwing
                    this->sprite->set_animation((this->sprite->animation % 8) + THROWING_ANIMATION);
                    this->sprite->set_frame(0);
                    this->sprite->interval_ms = 125;
                }

                break;

            case EDIBLE:
                // eat

                break;

            case WEAPON:
                this->is_busy = ATTACKING;

                // set animation to attacking
                this->sprite->set_animation((this->sprite->animation % 8) + ATTACKING_ANIMATION);
                this->did_miss = true;

                // attack the closest foe (if close enough)
                if (this->closest_foe != nullptr && this->closest_distance < 32.0f) {
                    this->did_miss = false;
                    this->closest_foe->attack(this->damage);
                }

                break;

            case ARMOUR:
                // equip

                break;

            default: break;
            }
        }

        // eat cheese
        if (controller != nullptr && controller->is_hit(ACTION2) && this->cheese > 0 && this->health < this->max_health) {
            this->cheese--;
            this->health++;

            if (this->health > this->max_health) {
                this->health = this->max_health;
            }

            this->is_busy = EATING;
            this->busy_ticks = game->ticks;

            // set animation to eating cheese
            this->sprite->set_animation(EATING_ANIMATION);
            this->sprite->interval_ms = 75;

            break;
        }

        break;

    case ATTACKING:
        // move using arrow keys
        if (controller != nullptr) {
            x_dir = int(controller->is_down(RIGHT)) - int(controller->is_down(LEFT));
            y_dir = int(controller->is_down(DOWN)) - int(controller->is_down(UP));
        }
        mov_speed = this->max_mov_speed / 2.0f;

        // will return to normal after << 500 ms >>
        if (game->ticks - this->busy_ticks > 500) {
            this->is_busy = FALSE;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }

        if (this->did_miss) {
            game->push_icon(MISS_ICON, this->dst_rect.x + 16.0f + game->corner_x, this->dst_rect.y + game->corner_y - 4.0f, &this->icon_src, &this->icon_dst);
        }

        break;

    case ATTACKED:
        // move according to the random throw direction set when attacked
        x_dir = this->throw_x;
        y_dir = this->throw_y;
        mov_speed = this->max_mov_speed;

        // will return to normal after << 250 ms >>
        if (game->ticks - this->busy_ticks > 250) {
            this->is_busy = FALSE;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }

        break;

    case THROWING:
        this->sprite->update_frame();

        // will return to normal after << 500 ms >>
        if (game->ticks - this->busy_ticks > 500) {
            this->is_busy = FALSE;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }

        break;

    case EATING:
        this->sprite->update_frame();

        // will return to normal after << 750 ms >>
        if (game->ticks - this->busy_ticks > 750) {
            this->is_busy = FALSE;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }

        break;
    }

    // new coordinates calculated with direction moving, movement speed, and diagonal multiplier (if necessary)
    float dx = float(x_dir) * (y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;
    float dy = float(y_dir) * (x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;

    dx = cnf_clamp(dx, -4.0f, 4.0f);
    dy = cnf_clamp(dy, -4.0f, 4.0f);

    float new_x = this->x + dx;
    float new_y = this->y + dy;

    // only move if there isn't a collider in the way
    if (!_is_collision(new_x, this->y)) this->x = new_x;
    if (!_is_collision(this->x, new_y)) this->y = new_y;

    // if moved onto a new tile, update the path finding
    int new_tile_x = (int)this->x / game->tile_width;
    int new_tile_y = (int)this->y / game->tile_height;
    if (new_tile_x != this->tile_x || new_tile_y != this->tile_y) {
        foe_path_find(this);
        this->tile_x = new_tile_x;
        this->tile_y = new_tile_y;
    }

    if (!this->is_feta) {
        this->dst_rect.x = (float)(int)((SCREEN_WIDTH / 2) - 16);
        this->dst_rect.y = (float)(int)((SCREEN_HEIGHT / 2) - 24);

        this->hat_dst.x = (float)(int)((SCREEN_WIDTH / 2) - 8);
        this->hat_dst.y = (float)(int)((SCREEN_HEIGHT / 2) - 24);

        game->set_view(this->x, this->y);

        // for (int ty = this->tile_y-8; ty < this->tile_y+9; ty++) {
        //     for (int tx = this->tile_x-8; tx < this->tile_x+9; tx++) {
        //         foe_debug_tile(tx, ty);
        //     }
        // }
    } else {
        this->dst_rect.x = this->x - float(16 + game->corner_x);
        this->dst_rect.y = this->y - float(24 + game->corner_y);

        this->hat_dst.x = this->x - float(8 + game->corner_x);
        this->hat_dst.y = this->y - float(24 + game->corner_y);
    }

    // display the sprite to the screen
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);

    // this->hat->set_animation(this->sprite->animation % 8);
    // game->push_sprite(this->hat->tex_id, this->hat->texture, &this->hat->frame, &this->hat_dst, 23);
}

// will be called by a foe
bool Mouse::attack(int damage) {
    // don't get attacked if was already attacked
    if (this->is_busy == ATTACKED)
        return false;

    this->is_busy = ATTACKED;
    this->busy_ticks = game->ticks;

    // set animation to attacked
    this->sprite->set_animation((this->sprite->animation % 8) + ATTACKED_ANIMATION);

    // apply armour to damage
    damage -= this->armour;
    if (damage > 0) {
        this->health -= damage;
    }

    // if dead, just revive
    if (this->health < 0) {
        this->health = this->max_health;
    }

    return true;
}

bool Mouse::push_item(const std::string &item_id) {
    if (!item_info.contains(item_id)) {
        return false;
    }

    Mouse::Item new_item;
    new_item.item_id = item_id;
    new_item.count = 1;

    if (this->items.empty()) {
        this->items.push_back(new_item);
        return true;
    }

    for (auto it = this->items.begin(); it != this->items.end(); ++it) {
        if (it->item_id == item_id) {
            it->count++;
            return true;
        } else if (it->item_id > item_id) {
            if (this->sel_item >= std::distance(this->items.begin(), it)) {
                this->sel_item++;
            }

            this->items.insert(it, new_item);

            return true;
        }
    }

    this->items.push_back(new_item);
    return true;
}

bool Mouse::remove_item(const std::string &item_id) {
    for (auto it = this->items.begin(); it != this->items.end(); it++) {
        if (it->item_id == item_id) {
            if (--it->count <= 0) {
                this->items.erase(it);
                return true;
            }

            return false;
        } else if (it->item_id > item_id) {
            return false;
        }
    }

    return false;
}

int Mouse::add_cheese(int amount) {
    int remaining_amount = this->max_cheese - this->cheese;

    if (remaining_amount < 1)
        return 0;

    if (remaining_amount < amount) {
        this->cheese += remaining_amount;
        return remaining_amount;
    }

    // amount < remaining_amount
    this->cheese += amount;
    return amount;
}

Mouse *closest_mouse(float x, float y, float min_distance) {
    float distance_cheddar = distance_between_points(x, y, cheddar->x, cheddar->y);
    float distance_feta = distance_between_points(x, y, feta->x, feta->y);

    if (distance_cheddar < distance_feta) {
        if (min_distance < 0.1f || distance_cheddar < min_distance) {
            return cheddar;
        }
    } else if (min_distance < 0.1f || distance_feta < min_distance) {
        return feta;
    }

    return nullptr; // no close-enough mouse
}
