#ifndef INVENTORY_OBJ
#define INVENTORY_OBJ "inventory"

#include "object.h"

#define NOTHING_EQUIPPED "---"

class Inventory : public Object {
public:
    Inventory();
    ~Inventory();

    void step() override;
    void save_data() override;
    void post_save_data() override;

    bool push_item(const std::string &item_id);
    int add_cheese(int amount);

    std::string attack_item = "";

private:
    void render_status_menu();
    void render_bag_menu();
    void render_item_menu();

    int cheese = 0, max_cheese = 3;
    std::vector<std::string> items;
    int max_items = 4;
    std::string equipped_weapon = NOTHING_EQUIPPED, equipped_armour = NOTHING_EQUIPPED;

    enum { NOT_DISPLAYING, BAG, ITEM } menu = NOT_DISPLAYING;
    int sel_item = 0;
    #define NUM_ACTION_OPTIONS 3
    int sel_action = 0;
};

class InventoryFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        return new Inventory();
    }
};

extern Inventory *inventory;

#endif
