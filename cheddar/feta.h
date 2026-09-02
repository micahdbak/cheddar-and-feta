#ifndef FETA_OBJ
#define FETA_OBJ "feta"

#include "object.h"
#include "hitbox.h"
#include "mouse.h"
#include "save_data.h"

#include <vector>

class Feta : public Mouse, public HitSource {
public:
    Feta(int x, int y, int animation);
    ~Feta();

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
    void signal_down() override {};

    // HitSource
    float hitsource_x() override { return this->x; }
    float hitsource_y() override { return this->y; }
    void hitsource_notify() override;

    void synchronize();

    // directly acted on by net_receiver
    // Mouse: x, y, is_down
    int frame_i = 0, animation = 0, which_emote = -1;
    std::vector<std::pair<std::string, std::string>> new_objects;
    bool synchronized = false;

private:
    int tile_x, tile_y;

    Sprite *sprite, *emotes;
    SDL_FRect dst_rect, emote_rect;

    std::vector<Game::HudItem> items;
};

class FetaFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y, animation;
        if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &animation)) FATAL_ERROR

        return new Feta(x, y, animation);
    }
};

#endif