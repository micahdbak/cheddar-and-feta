#include "controller.h"
#include "../foes/foe.h"
#include "game.h"
#include "items/item.h"
#include "textbox.h"

#include <iostream>

std::unordered_map<std::string, Item> item_info;

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
