#include "foes/foe.h"
#include "game.h"
#include "hurtbox.h"
#include "items/cheese.h"
#include "items/item.h"
#include "items/persister.h"
#include "items/tossed.h"
#include "mouse.h"
#include "net_agent.h"
#include "save_data.h"

#include "items/save.h"
#include "items/toothpick.h"

#include <iostream>
#include <cmath>

bool mice_locked = false;
bool _locked_due_to_loading = false;
Mouse *cheddar = nullptr, *feta = nullptr;

Mouse::Mouse(std::vector<Mouse::SpawnCoord> &coordinates, bool is_feta, std::string options) {
    this->is_feta = is_feta;

    if (!this->is_feta) {
        if (cheddar != nullptr) FATAL_ERROR
        cheddar = this;
        this->sprite = new Sprite("sprites/cheddar.bmp", 32, 32, 250);
        this->name = "Cheddar";

        // add feta to the map
        options[0] = '1';
        game->push_object(MOUSE_OBJ, options);
        game->push_object(ITEM_PERSISTER_OBJ, "");
    } else {
        if (feta != nullptr) FATAL_ERROR
        feta = this;
        this->sprite = new Sprite("sprites/feta.bmp", 32, 32, 250);
        this->name = "Feta";
    }

    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -8, -8, 16, 12));

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;

    if (save.geti(LOAD_SAVE) && save.has(this->name + MOUSE_X)) {
        this->x = save.getf(this->name + MOUSE_X);
        this->y = save.getf(this->name + MOUSE_Y);
        this->sprite->set_animation(save.geti(this->name + MOUSE_ANIMATION));
    } else {
        // SaveData::geti returns 0 if spawn_at is unset - this default value is ok
        int coord = save.geti(MOUSE_SPAWN_AT);
        if (coord < 0 || coord >= coordinates.size()) {
            coord = 0;
        }

        // feta loads second so unset this then
        if (this->is_feta) {
            save.puti(MOUSE_SPAWN_AT, 0); // reset to 0
        }

        if (coordinates.empty()) {
            coordinates.push_back(Mouse::SpawnCoord{0.0f, 0.0f, 0});
        }

        this->x = coordinates[coord].x;
        this->y = coordinates[coord].y;
        this->sprite->set_animation(coordinates[coord].animation);
    }

    std::string items_s = save.value(this->name + MOUSE_ITEMS);
    if (!items_s.empty()) {
        const char *arr = items_s.c_str();
        int i = 0;
        do {
            char item_id[256];
            int count = 1;
            sscanf(arr, "%[^*] * %d", item_id, &count);

            // validate item
            if (item_info.find(item_id) == item_info.end() || count <= 0)
                continue;

            Mouse::Item item{ item_id, count };
            this->items.push_back(item);

            while (*arr != '\0' && *arr != ',')
                arr++;

            if (*arr == ',')
                arr++;

            if (*arr == '\0')
                break;
        } while (i++ < 100);
    } else if (!is_feta) {
        this->items.push_back(Mouse::Item{ ITEM_SAVE, 1 });
    }

    this->tile_x = (int)this->x / game->tile_width;
    this->tile_y = (int)this->y / game->tile_height;
    foe_path_find(this);

    if (!this->is_feta) {
        game->set_view(this->x, this->y);
    }

    if (!this->is_feta) {
        this->dst_rect.x = (float)(int)((SCREEN_WIDTH / 2) - 16);
        this->dst_rect.y = (float)(int)((SCREEN_HEIGHT / 2) - 24);

        game->set_view(this->x, this->y);
    } else {
        this->dst_rect.x = this->x - float(16 + game->corner_x);
        this->dst_rect.y = this->y - float(24 + game->corner_y);
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

void Mouse::save_data() {
    save.putf(this->name + MOUSE_X, this->x);
    save.putf(this->name + MOUSE_Y, this->y);
    save.puti(this->name + MOUSE_ANIMATION, this->sprite->animation);

    std::string items_s = "";
    for (int i = 0; i < this->items.size(); i++) {
        if (this->items[i].count <= 0)
            continue;

        char item[256];
        snprintf(item, sizeof(item), "%s*%d", this->items[i].item_id.c_str(), this->items[i].count);
        items_s += item;

        if (i + 1 < this->items.size()) {
            items_s += ",";
        }
    }
    save.data[this->name + MOUSE_ITEMS] = items_s;
}

static bool _is_collision(float x, float y) {
    return game->point_in_collider(x+2.0f, y) ||
        game->point_in_collider(x-2.0f, y) ||
        game->point_in_collider(x, y-2.0f);
}

#define RUNPREP_ANIMATION   8
#define ATTACKING_ANIMATION 16
#define ATTACKED_ANIMATION  24
#define THROWING_ANIMATION  32
#define DOWN_ANIMATION      40
#define EATING_ANIMATION    48
#define DANCING_ANIMATION   49
#define SLEEPING_ANIMATION  50

void Mouse::step() {
    if (mice_locked) {
        game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
        return;
    }

    Controller *controller = this->is_feta ? &remote_controller : &local_controller;

    if (!this->items.empty()) {
        this->sel_item += controller->is_hit(Button::CYCLE_RIGHT) - controller->is_hit(Button::CYCLE_LEFT);
        if (this->sel_item < -1)
            this->sel_item = this->items.size() - 1;
        else if (this->sel_item > this->items.size() - 1)
            this->sel_item = -1;
    } else {
        this->sel_item = -1;
    }

    std::string sel_item_id = this->sel_item < 0 ? ITEM_NONE : this->items[this->sel_item].item_id;
    int sel_item_count = sel_item_id == ITEM_NONE ? 1 : this->items[this->sel_item].count;

    if (!this->is_feta)
        game->draw_hud(game->ui, sel_item_id, sel_item_count, this->health, this->max_health);

    // note that this is done after the above draw hud; using an item with count 0 = using no item
    if (sel_item_count == 0 && sel_item_id != ITEM_NONE) {
        sel_item_id = ITEM_NONE;
    }

    float mov_speed = 0.0f;
    int x_dir = 0, y_dir = 0;

    switch (this->is_busy) {
    case FALSE:
        // feta is sleeping when disconnected
        if (this->is_feta &&
            (game->net_state == NetworkAgent::State::NO_CONNECTION ||
            game->net_state == NetworkAgent::State::WAITING_FOR_PEER)) {
            this->sprite->set_animation(SLEEPING_ANIMATION);
            this->sprite->update_frame();
            this->sprite->interval_ms = 250;
            break; // don't do anything but sleep
        }

        // dancing
        if (controller->is_down(Button::DANCE)) {
            this->sprite->set_animation(DANCING_ANIMATION);
            this->sprite->update_frame();
            this->sprite->interval_ms = 200;

            break; // don't do anything but dance
        } else if (this->sprite->animation == DANCING_ANIMATION) {
            this->sprite->set_animation(0); // facing down
        }

        // move using arrow keys
        x_dir = int(controller->is_down(Button::RIGHT)) - int(controller->is_down(Button::LEFT));
        y_dir = int(controller->is_down(Button::DOWN)) - int(controller->is_down(Button::UP));

        // walk when Button::RUN is held down
        this->is_running = !controller->is_down(Button::RUN);

        // update sprite frame and animation only if moving
        if (x_dir != 0 || y_dir != 0) {
            this->sprite->set_animation(direction_from_dirs(x_dir, y_dir));
            this->sprite->update_frame();
        } else {
            // otherwise show only the first frame
            this->sprite->set_frame(0);
        }

        // running / walking
        if (x_dir == 0 && y_dir == 0) {
            mov_speed = 0.0f;
        } else if (this->is_running) {
            // if (game->ticks - this->running_ticks < 200) {
            //     mov_speed = 0;
            //     this->sprite->set_animation(RUNPREP_ANIMATION + direction_from_dirs(x_dir, y_dir));
            //     this->sprite->set_frame((game->ticks - this->running_ticks) / 50);
            // } else {
                mov_speed = this->max_mov_speed;
                this->sprite->interval_ms = 100;
            // }
        } else {
            mov_speed = this->max_mov_speed / 2.0f;
            this->sprite->interval_ms = 250;
        }

        if (item_info[sel_item_id].type == HELD_EFFECT) {
            mov_speed *= item_info[sel_item_id].speed;
        }

        // attack / use item
        if (controller->is_hit(Button::ATTACK)) {
            this->busy_ticks = game->ticks;

            if (x_dir == 0 && y_dir == 0)
                dirs_from_direction(this->sprite->animation % 8, &x_dir, &y_dir);

            switch (item_info[sel_item_id].type) {
            case USEFUL:
            case THROWABLE:
                char options[256];
                snprintf(options, sizeof(options), "%d,%d,%d,%d,%d", int(this->x), int(this->y), x_dir, y_dir, this->id);
                game->push_object(sel_item_id + USE_OBJ, std::string(options));
                this->remove_item(sel_item_id);

                if (item_info[sel_item_id].type == THROWABLE) {
                    this->is_busy = THROWING;

                    // set animation to throwing
                    this->sprite->set_animation((this->sprite->animation % 8) + THROWING_ANIMATION);
                    this->sprite->set_frame(0);
                    this->sprite->interval_ms = 125;
                }

                break;

            case EDIBLE:
                if (this->health < this->max_health) {
                    this->health++;
                    this->remove_item(sel_item_id);

                    this->is_busy = EATING;
                    this->busy_ticks = game->ticks;

                    // set animation to eating cheese
                    this->sprite->set_animation(EATING_ANIMATION);
                    this->sprite->interval_ms = 75;
                } else {
                    // play sfx for already full?
                }

                break;

            case HELD_EFFECT: {
                // must be an item which deals damage
                if (item_info[sel_item_id].damage < 1) {
                    break;
                }

                this->is_busy = ATTACKING;

                int x_off, y_off;
                HitBox::MakeOffset(x_dir, y_dir, &x_off, &y_off, 8.0f);
                HitBox::Properties props = {item_info[sel_item_id].damage, 200, 100};
                props.single_use = true;
                game->push_object(HITBOX_OBJ, HitBox::Options(this->id, this->id, x_off - 10, y_off - 12, 20, 20, props));

                // set animation to attacking
                this->sprite->set_animation((this->sprite->animation % 8) + ATTACKING_ANIMATION);
            } break;

            default: break;
            }
        } else if (controller->is_hit(Button::TOSS) && sel_item_id != ITEM_NONE && sel_item_id != ITEM_SAVE) {
            this->busy_ticks = game->ticks;

            if (x_dir == 0 && y_dir == 0)
                dirs_from_direction(this->sprite->animation % 8, &x_dir, &y_dir);

            // toss item
            game->push_object(TOSSED_ITEM_OBJ, TossedItem::Options(this->x, this->y, x_dir, y_dir, this->is_feta, sel_item_id));
            this->remove_item(sel_item_id);

            this->is_busy = THROWING;
            this->busy_ticks = game->ticks;

            // set animation to throwing
            this->sprite->set_animation((this->sprite->animation % 8) + THROWING_ANIMATION);
            this->sprite->set_frame(0);
            this->sprite->interval_ms = 125;
        }

        break;

    case ATTACKING: {
        // move using arrow keys
        if (controller != nullptr) {
            x_dir = int(controller->is_down(Button::RIGHT)) - int(controller->is_down(Button::LEFT));
            y_dir = int(controller->is_down(Button::DOWN)) - int(controller->is_down(Button::UP));
        }
        mov_speed = this->max_mov_speed / 2.0f;

        int cooldown = 250;

        if (this->did_hit)
            cooldown = 500;

        // will return to normal after << 250 or 500 ms >>
        if (game->ticks - this->busy_ticks > cooldown) {
            this->is_busy = FALSE;
            this->did_hit = false;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }
    } break;

    case ATTACKED:
        // move according to the random throw direction set when attacked
        x_dir = this->throw_x;
        y_dir = this->throw_y;
        mov_speed = MOUSE_DEFAULT_SPEED;

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

    case DOWNED:
        this->sprite->update_frame();
        this->sprite->interval_ms = 250;

        if (game->ticks - this->busy_ticks > 2000) {
            this->is_busy = FALSE;
            this->health = this->max_health / 2;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }
    }

    // you get 5 seconds after being "downed" before enemies will attack you again
    // (three seconds of movement)
    if (this->is_down && game->ticks - this->is_down_ticks > 5000) {
        this->is_down = false;
    }

    // new coordinates calculated with direction moving, movement speed, and diagonal multiplier (if necessary)
    if (mov_speed > 0.0f && (x_dir != 0 || y_dir != 0)) {
        float dx = float(x_dir) * (y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;
        float dy = float(y_dir) * (x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;

        dx = cnf_clamp(dx, -4.0f, 4.0f);
        dy = cnf_clamp(dy, -4.0f, 4.0f);

        float new_x = this->x + dx;
        float new_y = this->y + dy;

        bool collision_x = false, collision_y = false;

        if (x_dir != 0) {
            collision_x = _is_collision(new_x, this->y);
            if (!collision_x) this->x = new_x;
        }

        if (y_dir != 0) {
            collision_y = _is_collision(this->x, new_y);
            if (!collision_y) this->y = new_y;
        }

        // sliding on collision horizontally or vertically
        if (x_dir != 0 && y_dir == 0) {
            if (collision_x) {
                float new_y1 = this->y + fabs(dx);
                float new_y2 = this->y - fabs(dx);

                bool collision_y1 = _is_collision(this->x, new_y1);
                bool collision_y2 = _is_collision(this->x, new_y2);

                if (!collision_y1 && collision_y2) {
                    this->y = new_y1;
                } else if (collision_y1 && !collision_y2) {
                    this->y = new_y2;
                }
            }
        } else if (y_dir != 0 && x_dir == 0) {
            if (collision_y) {
                float new_x1 = this->x + fabs(dy);
                float new_x2 = this->x - fabs(dy);

                bool collision_x1 = _is_collision(new_x1, this->y);
                bool collision_x2 = _is_collision(new_x2, this->y);

                if (!collision_x1 && collision_x2) {
                    this->x = new_x1;
                } else if (collision_x1 && !collision_x2) {
                    this->x = new_x2;
                }
            }
        }

        // if moved onto a new tile, update the path finding
        int new_tile_x = (int)this->x / game->tile_width;
        int new_tile_y = (int)this->y / game->tile_height;
        if (new_tile_x != this->tile_x || new_tile_y != this->tile_y) {
            foe_path_find(this);
            this->tile_x = new_tile_x;
            this->tile_y = new_tile_y;
        }
    }

    if (!this->is_feta) {
        this->dst_rect.x = (float)(int)((SCREEN_WIDTH / 2) - 16);
        this->dst_rect.y = (float)(int)((SCREEN_HEIGHT / 2) - 24);

        game->set_view(this->x, this->y);

        #ifdef ONSCREEN_DEBUG
        for (int ty = this->tile_y-12; ty < this->tile_y+13; ty++) {
            for (int tx = this->tile_x-12; tx < this->tile_x+13; tx++) {
                foe_debug_tile(tx, ty);
            }
        }
        #endif
    } else {
        this->dst_rect.x = this->x - float(16 + game->corner_x);
        this->dst_rect.y = this->y - float(24 + game->corner_y);
    }

    // display the sprite to the screen
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
}

// will be called by a foe
void Mouse::attack(int damage) {
    // don't get attacked if was already attacked / down
    if (this->is_busy == ATTACKED || this->is_busy == DOWNED)
        return;

    this->is_busy = ATTACKED;
    this->busy_ticks = game->ticks;

    std::string sel_item_id = this->sel_item < 0 ? ITEM_NONE : this->items[this->sel_item].item_id;
    int armour = item_info[sel_item_id].armour;

    damage -= armour;
    if (damage < 0) {
        damage = 0;
    }

    // set animation to attacked
    this->sprite->set_animation((this->sprite->animation % 8) + ATTACKED_ANIMATION);

    // apply armour to damage
    if (damage > 0) {
        this->health -= damage;
    }

    // if dead, go down
    if (this->health <= 0) {
        this->health = 0;
        this->is_busy = DOWNED;
        this->is_down = true;
        this->busy_ticks = game->ticks;
        this->is_down_ticks = game->ticks;
        this->sprite->set_animation((this->sprite->animation % 8) + DOWN_ANIMATION);
    }
}

void Mouse::push_item(const std::string &item_id) {
    if (item_info.find(item_id) == item_info.end()) {
        return;
    }

    Mouse::Item new_item;
    new_item.item_id = item_id;
    new_item.count = 1;

    if (this->items.empty()) {
        this->items.push_back(new_item);
        return;
    }

    int i = 0;
    for (auto it = this->items.begin(); it != this->items.end(); ++it, ++i) {
        if (it->item_id == item_id) {
            it->count++;
            break;
        } else if (it->item_id > item_id) {
            this->items.insert(it, new_item);
            break;
        }
    }

    if (i >= this->items.size()) {
        this->items.push_back(new_item);
    }

    if (item_id != ITEM_CHEESE) {
        this->sel_item = i;
    }
}

void Mouse::remove_item(const std::string &item_id) {
    for (auto it = this->items.begin(); it != this->items.end(); it++) {
        if (it->item_id == item_id && it->count > 0) {
            it->count--;
            if (it->count == 0) {
                this->items.erase(it);
                this->sel_item = -1;
                break;
            }
        } else if (it->item_id > item_id) {
            return;
        }
    }
}

int Mouse::add_cheese(int amount) {
    for (int i = 0; i < amount; i++) {
        this->push_item(ITEM_CHEESE);
    }
    return amount;
}

Mouse *closest_mouse(float x, float y, float min_distance, bool forced) {
    float distance_cheddar = (cheddar->is_down && !forced) ? FLT_MAX : distance_between_points(x, y, cheddar->x, cheddar->y);
    float distance_feta = (feta->is_down && !forced) ? FLT_MAX : distance_between_points(x, y, feta->x, feta->y);

    if (distance_cheddar < distance_feta) {
        if (min_distance < 0.1f || distance_cheddar < min_distance) {
            return cheddar;
        }
    } else if (min_distance < 0.1f || distance_feta < min_distance) {
        return feta;
    }

    return nullptr; // no close-enough mouse
}
