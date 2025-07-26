#ifndef MOUSE_OBJ
#define MOUSE_OBJ "mouse"

#include <vector>
#include <iostream>

#include "controller.h"
#include "foes/foe.h"
#include "game.h"
#include "object.h"
#include "sprite.h"
#include "hitbox.h"

#define MOUSE_DEFAULT_SPEED 80.0f

// save keys
#define MOUSE_X         "_x"
#define MOUSE_Y         "_y"
#define MOUSE_ANIMATION "_animation"
#define MOUSE_ITEMS     "_items"
#define MOUSE_SPAWN_AT  DONT_WRITE "spawn_at"

class Mouse : public Object, public HitSource {
public:
    struct SpawnCoord {
        float x, y;
        int animation;
    };

    Mouse(std::vector<SpawnCoord> &coordinates, bool is_feta, std::string options);
    ~Mouse();

    void step() override;

    void save_data() override;

    bool attack(int damage);

    bool push_item(const std::string &item_id);
    void remove_item(const std::string &item_id);
    int add_cheese(int amount);

    float hitsource_x() override { return this->x; }
    float hitsource_y() override { return this->y; }

    void hitsource_notify() override {
        this->did_hit = true;
    }

    bool is_feta = true, is_down = false;
    float x, y;

    int throw_x = 0, throw_y = 0;

    std::string name = "Cheddar";

    int health = 10, max_health = 10;
    int damage = 1, armour = 0;

    struct Item {
        std::string item_id;
        int count;
    };

    std::vector<Item> items;
    int sel_item = -1;

    int cheese = 10, max_cheese = 10;

    float max_mov_speed = MOUSE_DEFAULT_SPEED;

private:
    SDL_FRect dst_rect, icon_src, icon_dst;
    Sprite *sprite;

    enum { FALSE, ATTACKING, ATTACKED, THROWING, EATING } is_busy = FALSE;
    Uint64 busy_ticks = 0;

    int tile_x, tile_y;

    bool did_hit = false;
};

#define MOUSEFACTORY_FAIL(str) {\
    std::cerr << "MouseFactory::create: bad options: " << (str) << std::endl;\
    std::exit(1);\
}

class MouseFactory : public ObjectFactory {
public:
    MouseFactory() {}
    ~MouseFactory() {}

    // options:
    // [is_feta] [x1,y1,a1] [x2,y2,a2]
    // e.g., 0 32,32,0 1600,640,2
    Object *create(const std::string &options) override {
        const char *arr = options.c_str();
        int is_feta;
        std::vector<Mouse::SpawnCoord> coordinates;

        if (sscanf(arr, "%d", &is_feta) < 1) MOUSEFACTORY_FAIL(options.c_str())
        if (*++arr == '\0' || *++arr == '\0') MOUSEFACTORY_FAIL(options.c_str())

        int i = 0;
        do {
            int x = 0, y = 0, animation = 0;
            if (sscanf(arr, "%d,%d,%d", &x, &y, &animation) < 3)
                break;

            Mouse::SpawnCoord coord;
            coord.x = (float)x;
            coord.y = (float)y;
            coord.animation = animation;
            coordinates.push_back(coord);

            while (*arr != '\0' && *arr != ' ')
                arr++;
            if (*arr == '\0') break;
            while (*++arr == ' ')
                arr++;
            if (*arr == '\0') break;
        } while (++i < 10); // max ten coords, incase something really breaks

        return new Mouse(coordinates, is_feta != 0, options);
    }
};

Mouse *closest_mouse(float x, float y, float min_distance);

extern bool mice_locked;
extern Mouse *cheddar, *feta;

#endif
