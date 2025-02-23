#ifndef INVENTORY_OBJ
#define INVENTORY_OBJ "inventory"

#include "object.h"

#include <vector>
#include <string>

#define NOTHING_EQUIPPED "---"

class Inventory : public Object {
public:
    Inventory();
    ~Inventory();

    void step() override;
    void save_data() override;
    void post_save_data() override;

    bool push_item(const std::string &item_id);
    bool remove_item(const std::string &item_id);
    int add_cheese(int amount);

    std::string cheddar_attack = "";
    std::string feta_attack = "";
    int cheese = 0, max_cheese = 3;

private:
    int n_attack_items();
    void render_cheddar_attack();
    void render_feta_attack();
    void render_status_menu();
    void render_bag_menu();
    void render_item_menu();

    std::vector<std::string> items;
    int max_items = 4;
    std::string equipped_weapon = NOTHING_EQUIPPED, equipped_armour = NOTHING_EQUIPPED;

    enum { NOT_DISPLAYING, BAG, ITEM } menu = NOT_DISPLAYING;
    int sel_cheddar_attack = 0, sel_feta_attack = 0;
    int sel_item = 0;
    #define NUM_ACTION_OPTIONS 3
    int sel_action = 0;

    Uint64 cycle_attack_item_ticks = 0;
};

class InventoryFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        return new Inventory();
    }
};

extern Inventory *inventory;

#endif
