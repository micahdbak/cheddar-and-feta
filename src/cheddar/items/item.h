#ifndef ITEM_H
#define ITEM_H

#include "../foes/foe.h"
#include "game.h"
#include "mouse.h"
#include "object.h"
#include "sprite.h"

#include <unordered_map>
#include <string>

#define ITEM_NONE   "item_none"
#define DROPPED_OBJ "_dropped"
#define USE_OBJ     "_use"

enum ItemType {
    USEFUL,
    THROWABLE,
    EDIBLE,
    WEAPON,
    ARMOUR
};

struct Item {
    ItemType type = USEFUL;
    std::string name = "Item";
    int damage = 0;
    int armour = 0;
};

extern std::unordered_map<std::string, Item> item_info;

class DroppedItem : public virtual Object {
public:
    static std::string Options(int x, int y) {
        char buff[256];
        snprintf(buff, sizeof(buff), "%d,%d", x, y);
        return std::string(buff);
    }

    DroppedItem(float x, float y, const char *sprite_path, int frame_w, int frame_h, int interval_ms);
    ~DroppedItem();

    void dropped_step();

    virtual void take(Mouse *mouse) = 0;

protected:
    float x, y;

    Sprite *sprite;

private:
    SDL_FRect dst_rect;
};

namespace UseItem {
    static std::string Options(int x, int y, int x_dir, int y_dir, int from_id) {
        char buff[256];
        snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d", x, y, x_dir, y_dir, from_id);
        return std::string(buff);
    }
}

#endif
