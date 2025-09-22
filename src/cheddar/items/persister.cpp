#include <iostream>

#include "game.h"
#include "item.h"
#include "persister.h"
#include "save_data.h"

ItemPersister *item_persister = nullptr;

ItemPersister::ItemPersister() {
    if (item_persister != nullptr) {
        std::cerr << "ItemPersister::ItemPersister: another instance exists" << std::endl;
        std::exit(1);
    }

    item_persister = this;

    std::string key = game->current_map + "_items";

    if (save.has(key)) {
        std::string items = save.data[key];
        const char *arr = items.c_str();

        char buff[256] = {0};

        while (*arr != '\0') {
            sscanf(arr, "%255[^;]", buff);

            arr += strlen(buff);
            if (*arr == ';')
                ++arr;

            char item_id[128];
            int item_x, item_y;
            sscanf(buff, "%127[^,],%d,%d", item_id, &item_x, &item_y);

            this->items_to_spawn.push(ItemSave{item_id, (float)item_x, (float)item_y});
        }
    }
}

ItemPersister::~ItemPersister() {
    item_persister = nullptr;
}

void ItemPersister::step() {
    while (!this->items_to_spawn.empty()) {
        ItemSave item = this->items_to_spawn.front();
        this->items_to_spawn.pop();

        std::string dropped_item_id = item.item_id;
        dropped_item_id += DROPPED_OBJ;
        game->push_object(dropped_item_id, DroppedItem::Options((float)item.x, (float)item.y));
    }
}

void ItemPersister::save_items() {
    std::string key = game->current_map + "_items";
    std::string data;

    auto it = this->items_to_save.begin();

    while (it != this->items_to_save.end()) {
        char buff[256];
        ItemPersister::ItemSave item = (*it).second;
        snprintf(buff, sizeof(buff), "%s,%d,%d", item.item_id.c_str(), (int)item.x, (int)item.y);
        data += buff;

        if (++it == this->items_to_save.end()) {
            break;
        }

        data += ";";
    }

    save.data[key] = data;
}

void ItemPersister::remember_item(const std::string &item_id, float x, float y, int id) {
    ItemPersister::ItemSave item;
    item.id = id;
    item.item_id = item_id;
    item.x = x;
    item.y = y;

    this->items_to_save[id] = item;
    this->save_items();
}

void ItemPersister::forget_item(int id) {
    this->items_to_save.erase(id);
    this->save_items();
}
