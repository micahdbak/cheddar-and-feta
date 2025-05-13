#ifndef ITEM_H
#define ITEM_H

#include "../foes/foe.h"
#include "game.h"
#include "mouse.h"
#include "object.h"
#include "sprite.h"

#include <unordered_map>
#include <string>

#define ITEM_NONE "item_none"
#define USE_OBJ   ".use"

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

#define DROPPED_OBJ ".dropped"

class DroppedItem : public Object {
public:
    DroppedItem(float x, float y, const char *sprite_path, int frame_w, int frame_h, int interval_ms);
    ~DroppedItem();

    void step() override;

    virtual bool take(Mouse *mouse) = 0;

    Sprite *sprite;

private:
    void render_choice();

    enum { IDLE, PROMPT, CHOICE, RESULT } state = IDLE;
    enum { TAKE, LEAVE } choice = TAKE;
    std::string unique_id;
    float x, y;
    SDL_FRect dst_rect;
};

// ---- thrown item ----

class ThrownItem : public Object {
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

#endif
