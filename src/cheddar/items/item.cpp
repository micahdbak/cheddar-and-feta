#include "controller.h"
#include "../foes/foe.h"
#include "game.h"
#include "items/item.h"
#include "items/persister.h"
#include "save_data.h"
#include "textbox.h"

#include <iostream>

std::unordered_map<std::string, Item> item_info;

DroppedItem::DroppedItem(std::string item_id, float x, float y, const char *sprite_path, int frame_w, int frame_h, int interval_ms) {
    this->item_id = item_id;
    this->x = x;
    this->y = y;

    this->sprite = new Sprite(sprite_path, frame_w, frame_h, interval_ms);
    this->dst_rect.w = float(frame_w);
    this->dst_rect.h = float(frame_h);

    if (game->creating_objects) {
        this->spawned_item = true;

        char buff[256];
        snprintf(buff, sizeof(buff), "%s_%d_%d", this->item_id.c_str(), (int)this->x, (int)this->y);
        std::string key = game->current_map + "_" + buff;

        // cancel creation if this save entry exists
        if (save.geti(key)) {
            game->delete_object = true;
        }
    } else if (item_persister != nullptr) {
        // item dropped by enemy/player
        item_persister->remember_item(item_id, x, y, this->id);
    }
}

DroppedItem::~DroppedItem() {
    if (!game->deleting_objects && item_persister != nullptr) {
        // item picked up by player, most likely
        item_persister->forget_item(this->id);
    }

    delete this->sprite;
    this->sprite = nullptr;
}

void DroppedItem::dropped_step() {
    Mouse *mouse = closest_mouse(this->x, this->y, 12.0f, true);
    if (!mice_locked && mouse != nullptr) {
        this->take(mouse);
        game->delete_object = true;

        // spawned item was taken; don't spawn next time
        if (this->spawned_item) {
            char buff[256];
            snprintf(buff, sizeof(buff), "%s_%d_%d", this->item_id.c_str(), (int)this->x, (int)this->y);
            std::string key = game->current_map + "_" + buff;
            save.puti(key, 1);
        }

        return;
    }

    if (this->sprite->interval_ms > 0)
        this->sprite->update_frame();

    this->dst_rect.x = this->x - (float)(int)(this->sprite->frame_w/2) - game->corner_x;
    this->dst_rect.y = this->y - (float)(int)(this->sprite->frame_h/2) - game->corner_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, this->sprite->frame_h/2);
}
