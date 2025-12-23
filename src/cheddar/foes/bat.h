#ifndef FOE_BAT_OBJ
#define FOE_BAT_OBJ "foe_bat"

#include "foe.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "sprite.h"

class FoeBat : public Foe, public HitSource {
public:
    FoeBat(int x, int y, int spawner_id);
    ~FoeBat();

    void action(Mouse *mouse) override;
    void attack_internal(int damage) override;

    void step() override;

    float hitsource_x() override { return this->x + this->off_x; }
    float hitsource_y() override { return this->y + this->off_y; }

    Foe::State prev_state = Foe::State::IDLE;

private:
    Sprite *sprite;
    SDL_FRect dst_rect, icon_src, icon_dst;
    Uint64 timer = 0, hurt_timer = 0, audio_timer = 0;
    int ticks_offset = 0, base_ticks_offset = 0;

    int max_health = 4, health = 4;
};

class FoeBatFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x = 0, y = 0, spawner_id = 0;
        if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id)) FATAL_ERROR

        return new FoeBat(x, y, spawner_id);
    }
};

#endif
