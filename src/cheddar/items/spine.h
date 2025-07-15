#ifndef ITEM_SPINE
#define ITEM_SPINE "item_spine"

#include "item.h"

class ThrownSpine : public TrackingItem {
public:
    ThrownSpine(float x, float y, int x_dir, int y_dir, int from_id);
    ~ThrownSpine();

    void step() override;

    void on_foe(Foe *foe) override {
        foe->attack(1);
        this->did_hit = true;
    }

    void on_mouse(Mouse *mouse) override {
        mouse->attack(1);
        mouse->throw_x = this->x_dir;
        mouse->throw_y = this->y_dir;
        this->did_hit = true;
    }

private:
    Sprite *sprite;
    SDL_FRect dst_rect;
    int x_dir, y_dir;
    Uint64 spawned_ticks;

    bool did_hit = false;
};

class ThrownSpineFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y, x_dir, y_dir, from_id;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_id);
        return new ThrownSpine(float(x), float(y), x_dir, y_dir, from_id);
    }
};

#endif