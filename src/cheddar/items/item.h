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
    int damage = 0, armour = 0;
};

extern std::unordered_map<std::string, Item> item_info;

// ---- dropped item ----

class DroppedItem : public virtual Object {
public:
    DroppedItem(float x, float y, const char *sprite_path, int frame_w, int frame_h, int interval_ms);
    ~DroppedItem();

    void dropped_step();

    virtual bool take(Mouse *mouse) = 0;

protected:
    float x, y;

    Sprite *sprite;

private:
    SDL_FRect dst_rect;
};

// ---- thrown item ----

class ThrownItem : public virtual Object {
public:
    ThrownItem(float x, float y, int x_dir, int y_dir);
    virtual ~ThrownItem() = default;

    void thrown_step();

    virtual void on_hit(Foe *foe) = 0;

    virtual void on_miss() = 0;

    static void ParseOptions(const std::string &options, float *x, float *y, int *x_dir, int *y_dir) {
        int _x, _y;
        sscanf(options.c_str(), "%d,%d,%d,%d", &_x, &_y, x_dir, y_dir);
        *x = float(_x);
        *y = float(_y);
    }

protected:
    float x, y;

private:
    int x_dir, y_dir;
    Uint64 thrown_ticks;
    int check_foe = 0;
};

// ---- tracking item ----

class TrackingItem : public virtual Object {
public:
    TrackingItem(float x, float y, float range, int from_id): x(x), y(y), range(range), from_id(from_id) {}
    ~TrackingItem() = default;

    // called on mouse/foe hit
    virtual void on_mouse(Mouse *mouse) = 0;
    virtual void on_foe(Foe *foe) = 0;

    void tracking_step(); // must be used in Object::step

protected:
    float x, y, range;

private:
    int check_foe = -1; // start at 0
    int from_id;
};

#endif
