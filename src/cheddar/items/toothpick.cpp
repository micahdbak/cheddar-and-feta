#include "toothpick.h"
#include "item.h"

#include <iostream>

ThrownToothpick::ThrownToothpick(float x, float y, int x_dir, int y_dir, int from_id)
    : x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
    Object *obj = game->get_object(from_id);
    Mouse *mouse;
    if (obj != nullptr && (mouse = dynamic_cast<Mouse*>(obj)) != nullptr) {
        // only drop if thrown by a mouse
        this->drop_item = true;
    } else {
        this->drop_item = false;
    }

    this->did_hit = false;

    this->sprite = new Sprite("sprites/item_toothpick.bmp", 16, 16, 0);
    this->sprite->set_animation(direction_from_dirs(x_dir, y_dir));
    this->dst_rect.w = 16.0f;
    this->dst_rect.h = 16.0f;

    this->spawned_ticks = game->ticks;

    HitBox::Properties props = {1, 1000, 1000};
    props.single_use = true;
    game->push_object(HITBOX_OBJ, HitBox::Options(this->id, from_id, -8, -8, 16, 16, props));
}

ThrownToothpick::~ThrownToothpick() {
    delete this->sprite;
    this->sprite = nullptr;
}

void ThrownToothpick::step() {
    float dx = float(this->x_dir) * (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
    float dy = float(this->y_dir) * (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
    float new_x = this->x + dx;
    float new_y = this->y + dy;

    if (this->did_hit || game->point_in_collider(new_x, new_y) || game->ticks - this->spawned_ticks > 1000) {
        game->delete_object = true;

        if (this->drop_item) {
            game->push_object(ITEM_TOOTHPICK DROPPED_OBJ, DroppedItem::Options(this->x, this->y));
        }

        return;
    }

    this->x = new_x;
    this->y = new_y;

    this->dst_rect.x = float(this->x - game->corner_x - 8);
    this->dst_rect.y = float(this->y - game->corner_y - 8);

    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 16);
}
