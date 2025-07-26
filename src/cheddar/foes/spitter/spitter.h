#ifndef FOE_SPITTER_OBJ
#define FOE_SPITTER_OBJ "spitter"

#include "../foe.h"
#include "../../hitbox.h"
#include "sprite.h"

class Spitter : public Foe {
public:
    Spitter(float x, float y, int spawner_id);
    ~Spitter();

    void action(Mouse *mouse) override;
    void attack_internal(int damage) override;

    void step() override;

    float get_x() { return this->x; }
    float get_y() { return this->y; }

    int displayed_direction = 0;

private:
    Uint64 timer, hurt_timer, direction_timer = 0;
    int last_direction = 0;
    int max_health = 10, health = 10;
    SDL_FRect icon_src, icon_dst;
};

class SpitterFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x, y, spawner_id;
        sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id);
        return new Spitter(x, y, spawner_id);
    }
};

#endif