#ifndef FOE_MOLE_OBJ
#define FOE_MOLE_OBJ "foe_mole"

#include "foe.h"
#include "hitbox.h"
#include "sprite.h"

class FoeMole : public Foe, public HitSource {
public:
    FoeMole(int x, int y, int spawner_id);
    ~FoeMole();

    void action(Mouse *mouse) override;
    void attack_internal(int damage) override;

    void step() override;

    float hitsource_x() override { return this->x + this->off_x; }
    float hitsource_y() override { return this->y + this->off_y; }

private:
    Sprite *sprite;
    SDL_FRect dst_rect, icon_src, icon_dst;
    Uint64 timer = 0, attack_timer = 0;

    int max_health = 10, health = 10;
};

class FoeMoleFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x = 0, y = 0, spawner_id = 0;
        sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id);
        return new FoeMole(x, y, spawner_id);
    }
};

#endif
