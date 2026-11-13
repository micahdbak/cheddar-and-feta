#include "persister.h"

#include <cstdio>
#include <cstring>
#include <string>

#include "item.h"
#include "object/entity.h"
#include "object/object.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "thoom/log.h"
#include "thoom/thoom.h"

ItemPersister* item_persister = nullptr;

ItemPersister::ItemPersister() : thoom::Object(nullptr) {
  if (item_persister != nullptr) {
    thoom_log_fatal("ItemPersister: another instance exists");
  }

  item_persister = this;

  std::string key = thoom::Scene::instance->map->path + "_items";

  if (thoom::save.has(key)) {
    std::string items = thoom::save.data[key];
    const char* arr = items.c_str();

    char buff[256] = {0};

    while (*arr != '\0') {
      sscanf(arr, "%255[^;]", buff);
      buff[255] = '\0';

      arr += strlen(buff);
      if (*arr == ';') ++arr;

      char item_id[128];
      int item_x, item_y;
      if (3 != sscanf(buff, "%127[^,],%d,%d", item_id, &item_x, &item_y)) {
        thoom_log_error("ItemPersister: bad save entry '%s'", buff);
        continue;
      }

      item_id[127] = '\0';

      this->items_to_spawn.push(
          ItemSave{item_id, (float)item_x, (float)item_y});
    }
  }
}

ItemPersister::~ItemPersister() { item_persister = nullptr; }

void ItemPersister::step() {
  if (this->items_to_spawn.empty()) {
    if (this->creating_items) this->creating_items = false;

    return;
  }

  this->creating_items = true;

  while (!this->items_to_spawn.empty()) {
    ItemSave item = this->items_to_spawn.front();
    this->items_to_spawn.pop();

    std::string dropped_item_id = item.item_id;
    dropped_item_id += DROPPED_OBJ;
    thoom::Scene::instance->create_entity(
        dropped_item_id, DroppedItem::options((float)item.x, (float)item.y),
        {thoom::Entity::Ownership::kRoundRobin, THOOM_PID_INVALID});
  }
}

void ItemPersister::save_items() {
  std::string key = thoom::Scene::instance->map->path + "_items";
  std::string data;

  auto it = this->items_to_save.begin();

  while (it != this->items_to_save.end()) {
    char buff[256];
    ItemPersister::ItemSave item = (*it).second;
    snprintf(buff, sizeof(buff), "%s,%d,%d", item.item_id.c_str(), (int)item.x,
             (int)item.y);
    data += buff;

    if (++it == this->items_to_save.end()) {
      break;
    }

    data += ";";
  }

  thoom::save.data[key] = data;
}

void ItemPersister::remember_item(const std::string& item_id, float x, float y,
                                  thoom_gid gid) {
  ItemPersister::ItemSave item;
  item.gid = gid;
  item.item_id = item_id;
  item.x = x;
  item.y = y;

  this->items_to_save[gid] = item;
  this->save_items();
}

void ItemPersister::forget_item(thoom_gid gid) {
  this->items_to_save.erase(gid);
  this->save_items();
}

thoom::Object* ItemPersister::create_object(const std::string& options) {
  return new ItemPersister();
}
