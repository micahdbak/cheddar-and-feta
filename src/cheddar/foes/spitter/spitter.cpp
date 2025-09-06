#include "abdomen.h"
#include "head.h"
#include "spitter.h"
#include "thorax.h"
#include "../../mouse.h"
#include "../../items/fire.h"

Spitter::Spitter(float x, float y, int spawner_id)
    : Foe(x, y, spawner_id, 500, 256.0f, 80.0f) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d", this->id);
    game->push_object(FOE_SPITTER_HEAD_OBJ, std::string(buff));
    game->push_object(FOE_SPITTER_THORAX_OBJ, std::string(buff));
    game->push_object(FOE_SPITTER_ABDOMEN_OBJ, std::string(buff));

    this->off_x = 0.0f;
    this->off_y = 0.0f;
}

Spitter::~Spitter() {
    // pass
}

void Spitter::action(Mouse *mouse) {
    this->timer = game->ticks;
}

void Spitter::attack_internal(int damage) {
    if (hurtbox_hitbox_id == HB_SHARED_FIRE) {
        return; // fire does no damage
    }

    this->health -= damage;

    if (this->health <= 0) {
        this->health = 0;
        this->state = Foe::State::DEAD;
        this->remove_from_foes();
    } else {
        this->state = Foe::State::HURT;
    }

    this->hurt_timer = game->ticks;
}

void Spitter::step() {
    this->foe_step();

    switch (this->state) {
    case Foe::State::ACTION: {
        Uint64 action_offset = game->ticks - this->timer;
        if (action_offset > 2000) {
            this->state = Foe::State::FORCE_RANDOM_TILE;
            break;
        }

        Mouse *mouse = closest_mouse(this->x, this->y, 80.0f, false);

        bool cancel_action = false;
        if (mouse == nullptr) {
            this->state = Foe::State::IDLE;
            break;
        } else {
            float distance = distance_between_points(this->x, this->y, mouse->x, mouse->y);
            if (distance < 32.0f) {
                this->tile_choice = AWAY;
                this->state = Foe::State::IDLE;
                break;
            }
        }

        int action_direction = 0;

        int x_dir, y_dir;
        dir_to_point(this->x, this->y, mouse->x, mouse->y, &x_dir, &y_dir);
        action_direction = direction_from_dirs(x_dir, y_dir);

        int direction_offset = 0;

        switch (action_offset / 250) {
        case 0: direction_offset = 0; break;
        case 1: direction_offset = -1; break;
        case 2: direction_offset = 0; break;
        case 3: direction_offset = 1; break;
        case 4: direction_offset = 0; break;
        case 5: direction_offset = -1; break;
        case 6: direction_offset = 0; break;
        case 7: direction_offset = 1; break;
        default: break;
        }

        this->direction = action_direction + direction_offset;
        if (this->direction < 0)
            this->direction = 7;
        else if (this->direction > 7)
            this->direction = 0;

        if (last_direction != this->direction && this->direction == this->displayed_direction) {
            last_direction = this->direction;

            int distance = (int)(x_dir != 0 && y_dir != 0 ? 16.0f * DIAG_MULTIPLIER : 16.0f);
            float fire_x = this->x + (float)(x_dir * distance);
            float fire_y = this->y + (float)(y_dir * distance);

            int fire_x_dir, fire_y_dir;
            dirs_from_direction(this->direction, &fire_x_dir, &fire_y_dir);
            game->push_object(ITEM_FIRE USE_OBJ, UseItem::Options(fire_x, fire_y, fire_x_dir * 2, fire_y_dir * 2, -1));
        }
    } break;

    case Foe::State::HURT:
        if (game->ticks - this->hurt_timer > 500) {
            this->state = Foe::State::WALKING;
        }

        game->push_health_bar(this->health, this->max_health, this->x, this->y - 16.0f, &this->icon_src, &this->icon_dst);

        break;

    case Foe::State::DEAD:
        if (game->ticks - this->hurt_timer > 2000) {
            // delete this object
            game->delete_object = true;
            return;
        }

        game->push_icon(SKULL_AND_BONES_ICON, this->x, this->y - 14.0f, &this->icon_src, &this->icon_dst);

        break;

    default:
        if (this->tile_choice == Foe::TileChoice::AWAY && this->current_distance >= 64.0f) {
            this->tile_choice = Foe::TileChoice::TOWARDS;
        }

        break;
    }

    int target_direction;
    if (this->tile_choice == Foe::TileChoice::AWAY) {
        target_direction = this->direction - 4;
        if (target_direction < 0)
            target_direction += 8;
    } else {
        target_direction = this->direction;
    }

    if (game->ticks - this->direction_timer > 100 && this->displayed_direction != target_direction) {
        this->direction_timer = game->ticks;

        int diff_up = (target_direction < this->displayed_direction ? target_direction + 8 : target_direction) - this->displayed_direction;
        int diff_down = this->displayed_direction - (target_direction > this->displayed_direction ? target_direction - 8 : target_direction);

        if (diff_up <= diff_down) {
            this->displayed_direction++;
        } else {
            // diff_down < diff_up
            this->displayed_direction--;
        }

        if (this->displayed_direction < 0)
            this->displayed_direction = 7;
        else if (this->displayed_direction > 7)
            this->displayed_direction = 0;
    }
}
