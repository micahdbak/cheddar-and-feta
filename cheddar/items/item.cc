#include "items/item.h"

#include <iostream>

#include "../foes/foe.h"
#include "audio_playback.h"
#include "controller.h"
#include "game.h"
#include "items/persister.h"
#include "save_data.h"
#include "textbox.h"

std::unordered_map<std::string, Item> item_info;

DroppedItem::DroppedItem(std::string item_id, float x, float y,
                         const char* sprite_path, int frame_w, int frame_h,
                         int interval_ms) {
  this->item_id = item_id;
  this->x = x;
  this->y = y;

  this->sprite = new thoom::Sprite(sprite_path, frame_w, frame_h, interval_ms);
  this->dst_rect.w = float(frame_w);
  this->dst_rect.h = float(frame_h);

  if (thoom::game->creating_objects) {
    this->spawned_item = true;

    char buff[256];
    snprintf(buff, sizeof(buff), "%s_%d_%d", this->item_id.c_str(),
             (int)this->x, (int)this->y);
    std::string key = thoom::game->current_map + "_" + buff;

    // cancel creation if this save entry exists
    if (thoom::save.geti(key)) {
      thoom::game->delete_object = true;
    }
  } else if (item_persister != nullptr) {
    item_persister->remember_item(item_id, x, y, this->id);

    if (!item_persister->creating_items) {
      thoom::play_audio("sfx/settle.wav", 0.5f, this->x, this->y, false);
    }
  }
}

DroppedItem::~DroppedItem() {
  if (!thoom::game->deleting_objects && item_persister != nullptr) {
    // item picked up by player, most likely
    item_persister->forget_item(this->id);
    thoom::play_audio("sfx/pickup.wav", 0.5f, this->x, this->y, false);
  }

  delete this->sprite;
  this->sprite = nullptr;
}

void DroppedItem::dropped_step() {
  Mouse* mouse = Mouse::closest_mouse(this->x, this->y, 12.0f, true);
  if (!mice_locked && mouse != nullptr) {
    this->take(mouse);
    thoom::game->delete_object = true;

    // spawned item was taken; don't spawn next time
    if (this->spawned_item) {
      char buff[256];
      snprintf(buff, sizeof(buff), "%s_%d_%d", this->item_id.c_str(),
               (int)this->x, (int)this->y);
      std::string key = thoom::game->current_map + "_" + buff;
      thoom::save.puti(key, 1);
    }

    return;
  }

  if (this->sprite->interval_ms > 0) this->sprite->update_frame();

  this->dst_rect.x = this->x - (float)(this->sprite->frame_w / 2);
  this->dst_rect.y = this->y - (float)(this->sprite->frame_h / 2);
  thoom::game->push_sprite(this->sprite->tex_id, this->sprite->texture,
                           &this->sprite->frame, &this->dst_rect,
                           this->sprite->frame_h / 2);
}
