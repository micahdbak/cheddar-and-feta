#include "audio_playback.h"
#include "cheddar.h"
#include "feta.h"
#include "game.h"
#include "hurtbox.h"
#include "items/cheese.h"
#include "items/item.h"
#include "items/persister.h"
#include "items/save.h"
#include "items/tossed.h"
#include "net_agent.h"
#include "save_data.h"

Cheddar::Cheddar(std::vector<Mouse::SpawnCoord> &coordinates, std::string options) {
    if (cheddar != nullptr) FATAL_ERROR
    cheddar = this;
    this->is_feta = false;

    this->sprite = new Sprite("sprites/cheddar.bmp", 32, 32, 250);

    std::string items_s = save.value(CHEDDAR_OBJ MOUSE_ITEMS);
    if (!items_s.empty()) {
        this->items = Mouse::read_items(items_s);
    } else {
        this->items.push_back(Game::HudItem{ ITEM_SAVE, 1 });
    }

    int feta_x = 0, feta_y = 0, feta_animation = 0;

    // if loading a save, read the location from the save file
    if (save.geti(LOAD_SAVE) && save.has(CHEDDAR_OBJ MOUSE_X)) {
        this->x = save.getf(CHEDDAR_OBJ MOUSE_X);
        this->y = save.getf(CHEDDAR_OBJ MOUSE_Y);
        this->sprite->set_animation(save.geti(CHEDDAR_OBJ MOUSE_ANIMATION));

        // don't set feta options, as feta will load from save as well
    } else {
        // get which coordinate to spawn at
        int coord = save.geti(MOUSE_SPAWN_AT);
        save.puti(MOUSE_SPAWN_AT, 0); // unset

        // validate
        if (coordinates.empty() || coord < 0 || coord >= coordinates.size()) FATAL_ERROR

        this->x = coordinates[coord].x;
        this->y = coordinates[coord].y;
        this->sprite->set_animation(coordinates[coord].animation);

        feta_x = coordinates[coord].x;
        feta_y = coordinates[coord].y;
        feta_animation = coordinates[coord].animation;
    }

    this->tile_x = (int)this->x / game->tile_width;
    this->tile_y = (int)this->y / game->tile_height;
    foe_path_find(this);

    game->set_view(this->x, this->y);

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;

    // make feta
    char feta_options[256];
    snprintf(feta_options, sizeof(feta_options), "%d,%d,%d", feta_x, feta_y, feta_animation);
    game->push_object(FETA_OBJ, std::string(feta_options));

    // push item persister and hurtbox
    game->push_object(ITEM_PERSISTER_OBJ, "");
    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -8, -8, 16, 12));

    this->is_down = false;
}

Cheddar::~Cheddar() {
    delete this->sprite;
    cheddar = nullptr;
}

void Cheddar::step() {
    if (mice_locked) {
        game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
        return;
    }

    set_listener(this->x, this->y); // for audio

    // ---- items ----

    if (!this->items.empty()) {
        if (local_controller.is_hit(Button::DIGIT)) {
            // -1 reserved for no item; 0+ for indexing into this->items
            // (note that the first number key is 1, not 0, hence subtract 2)
            int new_item = local_controller.digit - 2;

            if (new_item >= -1 && new_item < (int)this->items.size()) {
                this->sel_item = new_item;
            }
        }
    } else {
        // no item when no items (duh)
        this->sel_item = -1;
    }

    std::string sel_item_id;
    int sel_item_count;

    if (this->sel_item == -1) {
        sel_item_id = ITEM_NONE;
        sel_item_count = 1;
    } else {
        sel_item_id = this->items[this->sel_item].item_id;
        sel_item_count = this->items[this->sel_item].count;
    }

    game->draw_hud(game->ui, this->items, this->sel_item, this->health, this->max_health);

    // ---- state management ----
    
    float mov_speed = 0.0f;
    int x_dir = 0, y_dir = 0;

    switch (this->is_busy) {

    // ---- not busy ----

    case FALSE:
        // dancing
        if (local_controller.is_down(Button::DANCE)) {
            this->sprite->set_animation(DANCING_ANIMATION);
            this->sprite->update_frame();
            this->sprite->interval_ms = 200;
            break; // don't do anything but dance
        }

        // when no longer dancing, face down
        if (this->sprite->animation == DANCING_ANIMATION) {
            this->sprite->set_animation(0);
        }

        // move using arrow keys
        x_dir = int(local_controller.is_down(Button::RIGHT)) - int(local_controller.is_down(Button::LEFT));
        y_dir = int(local_controller.is_down(Button::DOWN)) - int(local_controller.is_down(Button::UP));

        if (x_dir != 0 || y_dir != 0) {
            this->sprite->set_animation(direction_from_dirs(x_dir, y_dir));

            if (this->sprite->update_frame() && (this->sprite->frame_i % 2) == 1) {
                play_audio("sfx/step.wav", 0.5, this->x, this->y, false);
            }

            mov_speed = this->max_mov_speed;
            this->sprite->interval_ms = 100;
        } else {
            mov_speed = 0.0f;
            dirs_from_direction(this->sprite->animation % 8, &x_dir, &y_dir);
            this->sprite->set_frame(0);
        }

        // handle an item which changes top speed
        if (item_info[sel_item_id].type == HELD_EFFECT) {
            mov_speed *= item_info[sel_item_id].speed;
        }

        if (local_controller.is_hit(Button::ATTACK)) {
            this->busy_ticks = game->ticks;

            switch (item_info[sel_item_id].type) {

            // ---- attack / use item ----

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

                    play_audio("sfx/throw.wav", 1.0f, this->x, this->y, false);
                }

                break;

            // ---- eat cheese ----

            case EDIBLE:
                if (this->health < this->max_health) {
                    this->health++;
                    this->remove_item(sel_item_id);

                    this->is_busy = EATING;
                    this->busy_ticks = game->ticks;

                    // set animation to eating cheese
                    this->sprite->set_animation(EATING_ANIMATION);
                    this->sprite->interval_ms = 75;

                    play_audio("sfx/cheese.wav", 1.0f, this->x, this->y, false);
                } else {
                    play_audio("sfx/full.wav", 1.0f, this->x, this->y, false);
                }

                break;

            // ---- held effect (*no item*, armour, boots) ----

            case HELD_EFFECT: {
                // if item doesn't deal damage, do nothing
                if (item_info[sel_item_id].damage < 1) {
                    break;
                }

                this->is_busy = ATTACKING;
                int x_off, y_off;
                HitBox::MakeOffset(x_dir, y_dir, &x_off, &y_off, 8.0f);
                HitBox::Properties props = {item_info[sel_item_id].damage, 200, 100};
                props.single_use = true;
                game->push_object(HITBOX_OBJ, HitBox::Options(this->id, this->id, x_off - 16, y_off - 18, 32, 32, props));

                // set animation to attacking
                this->sprite->set_animation((this->sprite->animation % 8) + ATTACKING_ANIMATION);

                play_audio("sfx/kick.wav", 1.0f, this->x, this->y, false);
            } break;

            default: break;
            }
        } else if (local_controller.is_hit(Button::TOSS) && sel_item_id != ITEM_NONE && sel_item_id != ITEM_SAVE) {
            this->busy_ticks = game->ticks;

            if (x_dir == 0 && y_dir == 0)
                dirs_from_direction(this->sprite->animation % 8, &x_dir, &y_dir);

            // toss item
            game->push_object(TOSSED_ITEM_OBJ, TossedItem::Options(this->x, this->y, x_dir, y_dir, false, sel_item_id));
            this->remove_item(sel_item_id);

            this->is_busy = THROWING;
            this->busy_ticks = game->ticks;

            // set animation to throwing
            this->sprite->set_animation((this->sprite->animation % 8) + THROWING_ANIMATION);
            this->sprite->set_frame(0);
            this->sprite->interval_ms = 125;

            play_audio("sfx/throw.wav", 1.0f, this->x, this->y, false);
        }

        break;

    // ---- attacking ----

    case ATTACKING: {
        // move slower using arrow keys
        x_dir = int(local_controller.is_down(Button::RIGHT)) - int(local_controller.is_down(Button::LEFT));
        y_dir = int(local_controller.is_down(Button::DOWN)) - int(local_controller.is_down(Button::UP));
        mov_speed = this->max_mov_speed / 2.0f;

        // longer cooldown when actually hit enemy
        int cooldown = this->did_hit ? 500 : 250;

        // will return to normal after << 250 or 500 ms >>
        if (game->ticks - this->busy_ticks > cooldown) {
            this->is_busy = FALSE;
            this->did_hit = false;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }
    } break;

    // ---- attacked ----

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

    // ---- throwing ----

    case THROWING:
        // move slower using arrow keys
        x_dir = int(local_controller.is_down(Button::RIGHT)) - int(local_controller.is_down(Button::LEFT));
        y_dir = int(local_controller.is_down(Button::DOWN)) - int(local_controller.is_down(Button::UP));
        mov_speed = this->max_mov_speed / 2.0f;

        this->sprite->interval_ms = 125;
        this->sprite->update_frame();

        // will return to normal after << 500 ms >>
        if (game->ticks - this->busy_ticks > 500) {
            this->is_busy = FALSE;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }

        break;

    // ---- eating ----

    case EATING:
        this->sprite->interval_ms = 75;
        this->sprite->update_frame();

        // will return to normal after << 750 ms >>
        if (game->ticks - this->busy_ticks > 750) {
            this->is_busy = FALSE;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }

        break;

    // ---- down ----

    case DOWNED:
        this->sprite->interval_ms = 250;
        this->sprite->update_frame();

        // will return to normal after << 2000 ms >>
        if (game->ticks - this->busy_ticks > 2000) {
            this->is_busy = FALSE;
            this->health = 5;

            // set animation to walking/running
            this->sprite->set_animation(this->sprite->animation % 8);
        }

        break;

    // ---- forced dancing (ending) ----

    case FORCED_DANCE:
        if (this->sprite->animation != DANCING_ANIMATION) {
            this->sprite->set_animation(DANCING_ANIMATION);
        }

        this->sprite->interval_ms = 200;
        this->sprite->update_frame();

        // will return to normal after << [this->dance_until] ms >>
        if (game->ticks - this->busy_ticks > this->dance_until) {
            this->is_busy = FALSE;
            this->sprite->set_animation(0);
        }

        break;
    }

    // you get 5 seconds after being "downed" before enemies will attack you again
    // (three seconds of movement)
    if (this->is_down && game->ticks - this->is_down_ticks > 5000) {
        this->is_down = false;
    }

    // ---- move cheddar ----

    // new coordinates calculated with direction moving, movement speed, and diagonal multiplier (if necessary)
    if (mov_speed > 0.0f && (x_dir != 0 || y_dir != 0)) {
        float dx = float(x_dir) * (y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;
        float dy = float(y_dir) * (x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;

        // prevents bad delta time movement (imagine a single frame lag spike)
        dx = cnf_clamp(dx, -4.0f, 4.0f);
        dy = cnf_clamp(dy, -4.0f, 4.0f);

        float new_x = this->x + dx;
        float new_y = this->y + dy;

        bool collision_x = false, collision_y = false;

        if (x_dir != 0) {
            collision_x = Mouse::check_collision(new_x, this->y);
            if (!collision_x) this->x = new_x;
        }

        if (y_dir != 0) {
            collision_y = Mouse::check_collision(this->x, new_y);
            if (!collision_y) this->y = new_y;
        }

        // sliding on collision horizontally or vertically
        if (x_dir != 0 && y_dir == 0) {
            if (collision_x) {
                float new_y1 = this->y + fabs(dx);
                float new_y2 = this->y - fabs(dx);

                bool collision_y1 = Mouse::check_collision(this->x, new_y1);
                bool collision_y2 = Mouse::check_collision(this->x, new_y2);

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

                bool collision_x1 = Mouse::check_collision(new_x1, this->y);
                bool collision_x2 = Mouse::check_collision(new_x2, this->y);

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

    // ---- display ----

    #ifdef ONSCREEN_DEBUG
    for (int ty = this->tile_y-12; ty < this->tile_y+13; ty++) {
        for (int tx = this->tile_x-12; tx < this->tile_x+13; tx++) {
            foe_debug_tile(tx, ty);
        }
    }
    #endif

    game->set_view(this->x, this->y);
    this->dst_rect.x = (float)(game->corner_x + SCREEN_WIDTH / 2) - 16.0f;
    this->dst_rect.y = (float)(game->corner_y + SCREEN_HEIGHT / 2) - 24.0f;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
}

void Cheddar::save_data() {
    save.putf(CHEDDAR_OBJ MOUSE_X, this->x);
    save.putf(CHEDDAR_OBJ MOUSE_Y, this->y);
    save.puti(CHEDDAR_OBJ MOUSE_ANIMATION, this->sprite->animation);
    save.data[CHEDDAR_OBJ MOUSE_ITEMS] = Mouse::encode_items(this->items);
}

void Cheddar::attack(int damage) {
    // don't get attacked if was already attacked / down
    if (this->is_busy == ATTACKED || this->is_busy == DOWNED || this->is_busy == FORCED_DANCE)
        return;

    this->is_busy = ATTACKED;
    this->busy_ticks = game->ticks;
    this->sprite->set_animation((this->sprite->animation % 8) + ATTACKED_ANIMATION);

    std::string sel_item_id;

    if (this->sel_item < 0 || this->sel_item >= this->items.size()) {
        sel_item_id = ITEM_NONE;
    } else {
        sel_item_id = this->items[this->sel_item].item_id;
    }

    int armour = item_info[sel_item_id].armour;

    damage -= armour;

    // nothing to do if damage is 0
    if (damage <= 0) {
        return;
    }

    this->health -= damage;
    play_audio("sfx/hurt.wav", 1.0f, this->x, this->y, false);

    // if dead, go down
    if (this->health <= 0) {
        this->health = 0;

        this->is_busy = DOWNED;
        this->busy_ticks = game->ticks;
        this->is_down_ticks = game->ticks;
        this->sprite->set_animation((this->sprite->animation % 8) + DOWN_ANIMATION);

        this->is_down = true;

        // game over if feta is down or disconnected
        if ((feta != nullptr && feta->is_down) ||
            game->net_state == NetworkAgent::State::NO_CONNECTION ||
            game->net_state == NetworkAgent::State::WAITING_FOR_PEER) {
            game->map = "maps/dead";
        }
    }
}

void Cheddar::push_item(const std::string &item_id) {
    if (item_info.find(item_id) == item_info.end()) FATAL_ERROR

    Game::HudItem new_item;
    new_item.item_id = item_id;
    new_item.count = 1;

    if (this->items.empty()) {
        this->items.push_back(new_item);
        return;
    }

    // see if the item is already in inventory, and increment count
    int i = 0;
    for (auto it = this->items.begin(); it != this->items.end(); ++it, ++i) {
        if (it->item_id == item_id) {
            it->count++;
            this->sel_item = i;
            return;
        }
    }

    // item not in inventory, push to back
    this->items.push_back(new_item);
    this->sel_item = i;
}

void Cheddar::push_cheese(int amount) {
    Game::HudItem new_item;
    new_item.item_id = ITEM_CHEESE;
    new_item.count = amount;

    if (this->items.empty()) {
        this->items.push_back(new_item);
        return;
    }

    // see if already has cheese, and increment by amount
    int i = 0;
    for (auto it = this->items.begin(); it != this->items.end(); ++it, ++i) {
        if (it->item_id == ITEM_CHEESE) {
            it->count += amount;
            return;
        }
    }

    // cheese not in inventory, push to back
    this->items.push_back(new_item);
}

void Cheddar::remove_item(const std::string &item_id) {
    for (auto it = this->items.begin(); it != this->items.end(); it++) {
        if (it->item_id == item_id && it->count > 0) {
            it->count--;
            if (it->count <= 0) {
                this->items.erase(it);
                this->sel_item = -1;
                break;
            }
        }
    }
}

void Cheddar::force_dance(Uint64 timeout_ms) {
    this->is_busy = Cheddar::Busy::FORCED_DANCE;
    this->dance_until = timeout_ms;
}

void Cheddar::set_throw(int throw_x, int throw_y) {
    this->throw_x = throw_x;
    this->throw_y = throw_y;
}

void Cheddar::set_max_mov_speed(float max_mov_speed) {
    this->max_mov_speed = max_mov_speed;
}

void Cheddar::signal_down() {
    // signal implies that feta is down; check if we are down too
    if (this->is_down && this->is_busy == DOWNED && feta != nullptr && feta->is_down) {
        game->map = "maps/dead";
    }
}