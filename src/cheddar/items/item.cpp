#include "controller.h"
#include "../foes/foe.h"
#include "game.h"
#include "items/item.h"
#include "textbox.h"

#include <iostream>

std::unordered_map<std::string, Item> item_info;

// ---- dropped item ----

DroppedItem::DroppedItem(float x, float y, const char *sprite_path, int frame_w, int frame_h, int interval_ms) {
    this->x = x;
    this->y = y;

    this->sprite = new Sprite(sprite_path, frame_w, frame_h, interval_ms);
    this->dst_rect.w = float(frame_w);
    this->dst_rect.h = float(frame_h);
}

DroppedItem::~DroppedItem() {
    delete this->sprite;
    this->sprite = nullptr;
}

void DroppedItem::dropped_step() {
    Mouse *mouse = closest_mouse(this->x, this->y, 8.0f);
    if (!mice_locked && mouse != nullptr && this->take(mouse)) {
        game->delete_object = true;
        return;
    }

    if (this->sprite->interval_ms > 0)
        this->sprite->update_frame();

    this->dst_rect.x = this->x - (float)(int)(this->sprite->frame_w/2) - game->corner_x;
    this->dst_rect.y = this->y - (float)(int)(this->sprite->frame_h/2) - game->corner_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, this->sprite->frame_h/2);

}

// ---- thrown item ----

ThrownItem::ThrownItem(float x, float y, int x_dir, int y_dir)
    : x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
    this->thrown_ticks = game->ticks;
}

void ThrownItem::thrown_step() {
    // initial delay
    if (game->ticks - this->thrown_ticks < 125)
        return;

    // cycle through available foes to find which is closest
    if (!foes.empty()) {
        this->check_foe++;
        if (this->check_foe >= foes.size())
            this->check_foe = 0;

        Foe *foe = foes[this->check_foe];
        if (foe != nullptr) {
            float distance = distance_between_points(this->x, this->y, foe->x, foe->y);

            // check if hit foe
            if (distance < 16.0f) {
                this->on_hit(foe);
                game->delete_object = true;
                return;
            }
        }
    }

    // throwable item expires after 1 s - 128 px
    if (game->ticks - this->thrown_ticks > 1125) {
        this->on_miss();
        game->delete_object = true;
        return;
    }

    float new_x = this->x + this->x_dir * (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
    float new_y = this->y + this->y_dir * (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;

    // miss on collision with wall
    if (game->point_in_collider(new_x, new_y)) {
        this->on_miss();
        game->delete_object = true;
    } else {
        this->x = new_x;
        this->y = new_y;
    }
}

// ---- tracking item ----

void TrackingItem::tracking_step() {
    if (!foes.empty()) {
        if (++check_foe == foes.size()) {
            check_foe = 0;
        }

        float dist = distance_between_points(this->x, this->y, foes[check_foe]->x, foes[check_foe]->y);
        if (dist < this->range && foes[check_foe]->id != this->from_id) {
            this->on_foe(foes[check_foe]);
            return;
        }
    }

    if (cheddar != nullptr) {
        float dist = distance_between_points(this->x, this->y, cheddar->x, cheddar->y);
        if (dist < this->range && cheddar->id != this->from_id) {
            this->on_mouse(cheddar);
            return;
        }
    }

    if (feta != nullptr) {
        float dist = distance_between_points(this->x, this->y, feta->x, feta->y);
        if (dist < this->range && feta->id != this->from_id) {
            this->on_mouse(feta);
            return;
        }
    }
}
