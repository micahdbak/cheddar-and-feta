#ifndef ITEM_H
#define ITEM_H

#include "enemy.h"
#include "game.h"
#include "object.h"
#include "sprite.h"

#include <map>
#include <string>

enum ItemType {
    USEFUL,
    EDIBLE,
    THROWABLE,
    WEAPON,
    ARMOUR
};

struct Item {
    ItemType type = USEFUL;
    std::string name = "Item";
    std::string description = "Unknown item.";
};

extern std::unordered_map<std::string, Item> item_info;

// ---- dropped item ----

#define DROPPED_OBJ ".dropped"

class DroppedItem : public Object {
public:
    DroppedItem(float x, float y, const char *sprite_path, int frame_w, int frame_h, int interval_ms);
    ~DroppedItem();

    void step() override;

    virtual std::string prompt_text() = 0;
    virtual std::string leave_text() = 0;
    virtual std::pair<bool, std::string> take_text() = 0;

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

#define THROWN_OBJ ".thrown"

class ThrownItem : public Object {
public:
    ThrownItem(float x, float y, int x_dir, int y_dir);
    virtual ~ThrownItem() = default;

    void thrown_step();

    virtual void on_hit(Enemy *enemy) = 0;

    virtual void on_miss() = 0;

    static void MakeOptions(char *options, size_t options_size, float x, float y, int x_dir, int y_dir) {
        snprintf(options, options_size, "%d,%d,%d,%d", int(x), int(y), x_dir, y_dir);
    }

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
    int check_enemy = 0;
};

#endif
