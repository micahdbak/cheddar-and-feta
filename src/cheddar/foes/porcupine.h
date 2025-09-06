#ifndef FOE_PORCUPINE_OBJ
#define FOE_PORCUPINE_OBJ "foe_porc"

#include "foe.h"
#include "sprite.h"

class FoePorcupine : public Foe {
public:
    FoePorcupine(int x, int y, int spawner_id);
    ~FoePorcupine();

    void action(Mouse *mouse) override;
    void attack_internal(int damage) override;

    void step() override;

private:
    Sprite *sprite;
    SDL_FRect dst_rect, icon_src, icon_dst;
    Uint64 timer = 0;

    int max_health = 10, health = 10;
    int spine_x_dir = 0, spine_y_dir = 0;
};

class FoePorcupineFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x = 0, y = 0, spawner_id = 0;
        sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id);
        return new FoePorcupine(x, y, spawner_id);
    }
};

#endif
