#ifndef GATE_OBJ
#define GATE_OBJ "gate"

#include <vector>

#include "object.h"
#include "sprite.h"
#include "game.h"

class FoeGate: public Object {
public:
    FoeGate(int x, int y, int spawner_id, int animation);
    ~FoeGate();

    void step() override;
private:
    float x, y;
    int colliders_offset, colliders_x, spawner_id;
    bool opened = false;

    Sprite *sprite;
    SDL_FRect dst_rect;

    std::vector<int> colliders;

    bool synchronized = false;
};

class FoeGateFactory: public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x = 0, y = 0, spawner_id = 0, animation = 0;
        if (4 != sscanf(options.c_str(), "%d,%d,%d,%d", &x, &y, &spawner_id, &animation)) FATAL_ERROR

        return new FoeGate(x, y, spawner_id, animation);
    }
};

#endif
