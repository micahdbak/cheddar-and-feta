#ifndef FOE_BUG_OBJ
#define FOE_BUG_OBJ "foe_bug"

#include "foe.h"
#include "hitbox.h"
#include "sprite.h"

class FoeBug : public Foe, public HitSource {
public:
    FoeBug(int x, int y, int spawner_id);
    ~FoeBug();

    void action(Mouse *mouse) override;
    void attack_internal(int damage) override;

    void step() override;

    float hitsource_x() override { return this->x + this->off_x; }
    float hitsource_y() override { return this->y + this->off_y; }

    Foe::State prev_state = Foe::State::IDLE;

private:
    Sprite *sprite;
    SDL_FRect dst_rect, icon_src, icon_dst;
    Uint64 timer = 0;

    int max_health = 4, health = 4;
};

class FoeBugFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x = 0, y = 0, spawner_id = 0;
        if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id)) FATAL_ERROR

        return new FoeBug(x, y, spawner_id);
    }
};

#endif
