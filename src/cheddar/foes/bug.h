#ifndef FOE_BUG_OBJ
#define FOE_BUG_OBJ "foe_bug"

#include "foe.h"
#include "sprite.h"

class FoeBug : public Foe {
public:
    FoeBug(int x, int y, int spawner_id);
    ~FoeBug();

    void action(Mouse *mouse) override;
    void attack(int damage) override;

    void step() override;

private:
    Sprite *sprite;
    SDL_FRect dst_rect;
    Uint64 timer = 0;

    int max_health = 2, health = 2;
};

class FoeBugFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x = 0, y = 0, spawner_id = 0;
        sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id);
        return new FoeBug(x, y, spawner_id);
    }
};

#endif
