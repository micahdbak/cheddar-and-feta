#ifndef ITEM_PERSISTER_OBJ
#define ITEM_PERSISTER_OBJ "item_persister"

#include <unordered_map>
#include <queue>

#include "object.h"

class ItemPersister : public Object {
public:
    ItemPersister();
    ~ItemPersister();

    void step() override;

    void remember_item(const std::string &item_id, float x, float y, int id);
    void forget_item(int id);

private:
    void save_items();

    struct ItemSave {
        std::string item_id;
        float x, y;
        int id;
    };

    std::unordered_map<int, ItemSave> items_to_save;
    std::queue<ItemSave> items_to_spawn;
};

class ItemPersisterFactory : public ObjectFactory {
public:
    Object *create(const std::string &) override {
        return new ItemPersister();
    }
};

extern ItemPersister *item_persister;

#endif