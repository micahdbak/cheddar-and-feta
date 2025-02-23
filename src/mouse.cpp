#include "inventory.h"
#include "item.h"
#include "mouse.h"
#include "save_data.h"

#include "item_toothpick.h"

#include <iostream>
#include <cmath>

bool mice_locked = false;
Mouse *cheddar = nullptr, *feta = nullptr;

Mouse::Mouse(int x, int y, bool is_feta) {
    this->is_feta = is_feta;
    if (!this->is_feta) {
        if (cheddar != nullptr) {
            std::cerr << "Mouse::Mouse error: another Cheddar exists..?" << std::endl;
            exit(1);
        }
        cheddar = this;

        // add the inventory handler to the map
        game->push_object(INVENTORY_OBJ, "");

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

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;

    this->x = float(x);
    this->y = float(y);

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
    if (this->is_feta) {
        cheddar = nullptr;
    } else {
        feta = nullptr;
    }
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

void Mouse::step() {
    Controller *controller = this->is_feta ? player2 : player1;
    if (mice_locked) {
        game->push_sprite(this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
        return;
    }

    // cycle through available enemies to find which is closest
    if (!enemies.empty()) {
        this->check_enemy++;
        if (this->check_enemy >= enemies.size())
            this->check_enemy = 0;

        Enemy *enemy = enemies[this->check_enemy];
        if (enemy != nullptr) {
            float enemy_x = enemy->x + float(game->tile_width/2);
            float enemy_y = enemy->y + float(game->tile_height/2);

            // calculate direction to enemy
            int x_dir, y_dir;
            dir_to_point(this->x, this->y, enemy_x, enemy_y, &x_dir, &y_dir);
            int enemy_direction = direction_from_dirs(x_dir, y_dir);

            // current direction that the player is facing
            int facing_direction = this->sprite->animation % 8;

            // if the enemy being checked is the previously determined closest enemy
            if (enemy == this->closest_enemy) {
                // if no longer facing this enemy
                if (facing_direction != enemy_direction) {
                    // then set the closest enemy to nullptr and wait to find the next closest enemy in this direction
                    this->closest_enemy = nullptr;
                    this->closest_distance = FLT_MAX;
                } else {
                    // if still facing it, update its distance, incase it has moved further away
                    this->closest_distance = distance_between_points(enemy_x, enemy_y, this->x, this->y);
                }
            } else if (facing_direction == enemy_direction) {
                // this enemy is not the closest enemy; check its distance
                float distance = distance_between_points(enemy_x, enemy_y, this->x, this->y);

                // if it is closer than the closest enemy, update it to become the closest enemy
                if (distance < this->closest_distance) {
                    this->closest_enemy = enemy;
                    this->closest_distance = distance;
                }
            }
        }
    }

    float mov_speed = 0.0f;
    int x_dir = 0, y_dir = 0;

    switch (this->is_busy) {
    case FALSE:
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

        mov_speed = 32.0f;
        this->sprite->interval_ms = 250;

        // run
        if (controller != nullptr && controller->is_down(L2)) {
            mov_speed = 64.0f;
            this->sprite->interval_ms = 100;
        }

        // attack
        if (controller != nullptr && controller->is_hit(ACTION1)) {
            this->busy_ticks = game->ticks;

            // if we have an attack item selected and it is still in the inventory
            if (!this->attack_item.empty() && inventory->remove_item(this->attack_item)) {
                if (x_dir == 0 && y_dir == 0)
                    dirs_from_direction(this->sprite->animation % 8, &x_dir, &y_dir);

                this->is_busy = THROWING;

                // set animation to throwing
                this->sprite->set_animation((this->sprite->animation % 8) + THROWING_ANIMATION);
                this->sprite->set_frame(0);
                this->sprite->interval_ms = 125;

                // create thrown item
                char options[256];
                ThrownItem::MakeOptions(options, sizeof(options), this->x, this->y, x_dir, y_dir);
                game->push_object(this->attack_item + THROWN_OBJ, std::string(options));
                this->attack_item = "";
            } else {
                this->attack_item = ""; // incase the second check of the previous if fails
                this->is_busy = ATTACKING;

                // set animation to attacking
                this->sprite->set_animation((this->sprite->animation % 8) + ATTACKING_ANIMATION);

                // attack the closest enemy (if close enough)
                if (this->closest_enemy != nullptr && this->closest_distance < 32.0f) {
                    this->closest_enemy->attack(this->damage);

                    // check if enemy just died
                    if (this->closest_enemy->dead) {
                        // set these accordingly so next attack is not the dead enemy
                        this->closest_enemy = nullptr;
                        this->closest_distance = FLT_MAX;
                    }
                }
            }

            break;
        }

        // eat cheese
        if (controller != nullptr && controller->is_hit(R1) && inventory->cheese > 0) {
            inventory->cheese--;
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
        mov_speed = 32.0f;

        // will return to normal after << 500 ms >>
        if (game->ticks - this->busy_ticks > 500) {
            this->is_busy = FALSE;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }

        break;

    case ATTACKED:
        // move according to the random throw direction set when attacked
        x_dir = this->throw_x;
        y_dir = this->throw_y;
        mov_speed = 64.0f;

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
    float new_x = this->x + float(x_dir) * (y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;
    float new_y = this->y + float(y_dir) * (x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;

    // only move if there isn't a collider in the way
    if (!_is_collision(new_x, this->y)) this->x = new_x;
    if (!_is_collision(this->x, new_y)) this->y = new_y;

    this->dst_rect.x = this->x - float(16 + game->corner_x);
    this->dst_rect.y = this->y - float(24 + game->corner_y);

    // only set game view if this mouse is being controlled right now
    if (!this->is_feta) {
        // this->dst_rect.x = float(SCREEN_WIDTH/2 - 16);
        // this->dst_rect.y = float(SCREEN_HEIGHT/2 - 24);

        game->set_view((int(this->x) + int(feta->x)) / 2, (int(this->y) + int(feta->y)) / 2);
    }

    // display the sprite to the screen
    game->push_sprite(this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
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

// will be called by an enemy
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

void Mouse::push_item(const std::string &item_id) {
    if (this->items.size() >= this->max_items || !item_info.contains(item_id)) {
        return;
    }

    this->items.push_back(item_id);
}

Mouse *closest_mouse(float x, float y, float min_distance) {
    float distance_cheddar = distance_between_points(x, y, cheddar->x, cheddar->y);
    float distance_feta = distance_between_points(x, y, feta->x, feta->y);

    if (distance_cheddar < distance_feta) {
        if (distance_cheddar < min_distance) {
            return cheddar;
        }
    } else if (distance_feta < min_distance) {
        return feta;
    }

    return nullptr; // no close-enough mouse
}
