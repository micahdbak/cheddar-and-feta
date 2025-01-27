#include "game.h"
#include "keyboard.h"
#include "mouse.h"
#include "save_data.h"

#include <iostream>

Mouse *mouse = nullptr;

Mouse::Mouse(int x, int y) {
    if (mouse != nullptr) {
        std::cerr << "Mouse::Mouse error: another mouse exists..?" << std::endl;
        exit(1);
    }
    mouse = this;

    this->sprite = new Sprite("sprites/mouse.bmp", 24, 24, 250);

    this->dst_rect.x = float(SCREEN_WIDTH/2 - 12);
    this->dst_rect.y = float(SCREEN_HEIGHT/2 - 20);
    this->dst_rect.w = 24.0f;
    this->dst_rect.h = 24.0f;

    this->x = float(x);
    this->y = float(y);

    if (save.data["mouse_data"] == "true") {
        save.data["mouse_data"] = "false";
        this->x = str_to_float(save.data["mouse_x"].c_str());
        this->y = str_to_float(save.data["mouse_y"].c_str());
        this->sprite->set_animation(atoi(save.data["mouse_animation"].c_str()));
    }

    game->set_view(this->x, this->y);
}

Mouse::~Mouse() {
    delete this->sprite;
    this->sprite = nullptr;
    mouse = nullptr;
}

static bool _is_collision(float x, float y) {
    return game->point_in_collider(x+2.0f, y) ||
        game->point_in_collider(x-2.0f, y) ||
        game->point_in_collider(x, y-2.0f);
}

#define ATTACKING_ANIMATION 8
#define ATTACKED_ANIMATION  16

void Mouse::step() {
    if (this->locked) {
        game->push_sprite(this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
        return;
    }

    // cycle through available enemies to find which is closest
    if (!enemies.empty()) {
        this->check_enemy++;
        if (this->check_enemy >= enemies.size())
            this->check_enemy = 0;

        Enemy *enemy = enemies[this->check_enemy];
        if (enemy == nullptr) return;

        float distance = distance_between_points(enemy->x + float(game->tile_width/2), enemy->y + float(game->tile_height/2), this->x, this->y);

        if (enemy == this->closest_enemy) {
            // update the distance to the currently closest enemy (incase it's further)
            this->closest_distance = distance;
        } else if (distance < this->closest_distance) {
            // new closets enemy; update distance and enemy pointer
            this->closest_distance = distance;
            this->closest_enemy = enemy;
        }
    }

    // if attacking or attacked, unset the animation when the interval has passed
    if (this->is_attack != FALSE) {
        // 100ms cooldown when attacked, 250ms cooldown when attacking
        if (game->ticks - this->attack_ticks > (this->is_attack == ATTACKED ? 100 : 250)) {
            this->is_attack = FALSE;
            this->sprite->set_animation(this->sprite->animation % 8);
        }
    } else if (keyboard.is_hit(SDLK_SPACE)) {
        this->is_attack = ATTACKING;
        this->attack_ticks = game->ticks;

        // set animation to attacking
        this->sprite->set_animation((this->sprite->animation % 8) + ATTACKING_ANIMATION);

        if (this->closest_enemy != nullptr && this->closest_distance < 32.0f) {
            this->closest_enemy->attack();
        }
    }

    // set movement speed
    float mov_speed;
    if (this->is_attack == ATTACKED) {
        mov_speed = 0.0f; // no movement when attacked - you're floored
    } else if (this->is_attack == FALSE && keyboard.is_down(SDLK_LSHIFT)) {
        mov_speed = 64.0f;
        this->sprite->set_interval_ms(100);
    } else {
        mov_speed = 32.0f;
        this->sprite->set_interval_ms(250);
    }

    // move using arrow keys
    int x_dir = int(keyboard.is_down(SDLK_RIGHT)) - int(keyboard.is_down(SDLK_LEFT));
    int y_dir = int(keyboard.is_down(SDLK_DOWN)) - int(keyboard.is_down(SDLK_UP));

    // new coordinates calculated with movement speed and diagonal multiplier
    float new_x = this->x + float(x_dir) * (y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;
    float new_y = this->y + float(y_dir) * (x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed * game->delta;

    // only move if there isn't a collider in the way
    if (!_is_collision(new_x, this->y)) this->x = new_x;
    if (!_is_collision(this->x, new_y)) this->y = new_y;

    // set the game's view
    game->set_view(this->x, this->y);

    // if not attacking or attacked, update frame only when moving;
    // otherwise set to first frame (standing)
    if (this->is_attack == FALSE) {
        if (x_dir != 0 || y_dir != 0) {
            this->sprite->update_frame();
        } else {
            this->sprite->set_frame(0);
        }

        // set the current animation with respect to the current arrow keys pressed
        if (x_dir > 0) {
            this->sprite->set_animation(2 - y_dir);
        } else if (x_dir < 0) {
            this->sprite->set_animation(6 + y_dir);
        } else if (y_dir > 0) {
            this->sprite->set_animation(0);
        } else if (y_dir < 0) {
            this->sprite->set_animation(4);
        }
    }

    // display the sprite to the screen
    game->push_sprite(this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
}

// will be called by an enemy
bool Mouse::attack() {
    // don't get attacked if was already attacked
    if (this->is_attack == ATTACKED)
        return false;

    this->is_attack = ATTACKED;
    this->attack_ticks = game->ticks;

    // set animation to attacked
    this->sprite->set_animation((this->sprite->animation % 8) + ATTACKED_ANIMATION);

    return true;
}

void Mouse::save_data() {
    save.data["mouse_data"] = "true";
    char buff[100];
    float_to_str(this->x, buff, sizeof(buff));
    save.data["mouse_x"] = buff;
    float_to_str(this->y, buff, sizeof(buff));
    save.data["mouse_y"] = buff;
    snprintf(buff, sizeof(buff), "%d", int(this->sprite->animation));
    save.data["mouse_animation"] = buff;
}

void Mouse::post_save_data() {
    save.data["mouse_data"] = "false";
}
