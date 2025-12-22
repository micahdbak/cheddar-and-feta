#ifndef CHEDDAR_OBJ
#define CHEDDAR_OBJ "cheddar"

#include "object.h"
#include "hitbox.h"
#include "mouse.h"

class Cheddar : public Mouse, public HitSource {
public:
    Cheddar(std::vector<Mouse::SpawnCoord> &coordinates, std::string options);
    ~Cheddar();

    // Object
    void step() override;
    void save_data() override;

    // Mouse
    void attack(int damage) override;
    void push_item(const std::string &item_id) override;
    void push_cheese(int amount) override;
    void remove_item(const std::string &item_id) override;
    void force_dance(Uint64 timeout_ms) override;
    void set_throw(int throw_x, int throw_y) override;
    void set_max_mov_speed(float max_mov_speed) override;
    void signal_down() override;

    // HitSource
    float hitsource_x() override        { return this->x; }
    float hitsource_y() override        { return this->y; }
    void hitsource_notify() override    { this->did_hit = true; }

private:
    int tile_x, tile_y;

    Sprite *sprite;
    SDL_FRect dst_rect;

    int health = 10, max_health = 10;
    std::vector<Game::HudItem> items;
    int sel_item = -1;

    int throw_x = 0, throw_y = 0;
    float max_mov_speed = MOUSE_DEFAULT_SPEED;

    enum Busy { FALSE, ATTACKING, ATTACKED, THROWING, EATING, DOWNED, FORCED_DANCE } is_busy = Cheddar::Busy::FALSE;
    Uint64 busy_ticks = 0, is_down_ticks = 0, dance_until = 0;
    bool did_hit = false;
};

class CheddarFactory : public ObjectFactory {
public:
    // options:
    // [x1,y1,a1] [x2,y2,a2]
    // e.g., 32,32,0 1600,640,2
    Object *create(const std::string &options) override {
        const char *arr = options.c_str();
        std::vector<Mouse::SpawnCoord> coordinates;

        int i = 0;
        do {
            int x = 0, y = 0, animation = 0;
            if (3 != sscanf(arr, "%d,%d,%d", &x, &y, &animation))
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

        return new Cheddar(coordinates, options);
    }
};

#endif