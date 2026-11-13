#include "items/item.h"

#include <cstdio>
#include <string>
#include <unordered_map>

#include "audio/audio.h"
#include "game_hud.h"
#include "items/persister.h"
#include "mouse.h"
#include "object/entity.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/log.h"

std::unordered_map<std::string, Item> item_info;

bool require_item(const GameHud::Item& item) {
  if (item_info.find(item.item_id) == item_info.end() || item.count <= 0) {
    thoom_log_fatal("require_item: bad item '%s'", item.item_id.c_str());
    return false;
  }

  return true;
}

DroppedItem::DroppedItem(const thoom::Entity::Context& context,
                         std::string item_id, float x, float y,
                         const char* sprite_path, int frame_w, int frame_h,
                         int interval_ms)
    : thoom::Entity(context) {
  this->item_id = item_id;
  this->x = x;
  this->y = y;

  this->sprite = new thoom::Sprite(sprite_path, frame_w, frame_h, interval_ms);
  this->dst_rect.w = float(frame_w);
  this->dst_rect.h = float(frame_h);

  if ((thoom::Scene::instance->state == thoom::Scene::kLoading)) {
    this->spawned_item = true;

    char buff[256];
    snprintf(buff, sizeof(buff), "%s_%d_%d", this->item_id.c_str(),
             (int)this->x, (int)this->y);
    std::string key = thoom::Scene::instance->map->path + "_" + buff;

    // cancel creation if this save entry exists
    if (thoom::save.geti(key)) {
      this->deleted = true;
    }
  } else if (item_persister != nullptr) {
    item_persister->remember_item(item_id, x, y, this->gid);

    if (!item_persister->creating_items) {
      thoom::Audio::instance->play("sfx/settle.wav", 0.5f, this->x, this->y,
                                   false);
    }
  }
}

DroppedItem::~DroppedItem() {
  if (!(thoom::Scene::instance->state == thoom::Scene::kUnloading) &&
      item_persister != nullptr) {
    // item picked up by player, most likely
    item_persister->forget_item(this->gid);
    thoom::Audio::instance->play("sfx/pickup.wav", 0.5f, this->x, this->y,
                                 false);
  }

  delete this->sprite;
  this->sprite = nullptr;
}

void DroppedItem::step() {
  Mouse* mouse = Mouse::closest_mouse(this->x, this->y, 12.0f, true);
  if (mouse != nullptr) {
    this->take(mouse);
    this->deleted = true;

    // spawned item was taken; don't spawn next time
    if (this->spawned_item) {
      char buff[256];
      snprintf(buff, sizeof(buff), "%s_%d_%d", this->item_id.c_str(),
               (int)this->x, (int)this->y);
      std::string key = thoom::Scene::instance->map->path + "_" + buff;
      thoom::save.puti(key, 1);
    }

    return;
  }

  if (this->sprite->interval_ms > 0) this->sprite->update_frame();
}

void DroppedItem::render() {
  this->dst_rect.x = this->x - (float)(this->sprite->frame_w / 2);
  this->dst_rect.y = this->y - (float)(this->sprite->frame_h / 2);
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, this->sprite->frame_h / 2);
}
