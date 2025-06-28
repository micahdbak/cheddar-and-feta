#ifndef BELL_OBJ
#define BELL_OBJ "bell"

#include "object.h"
#include "sprite.h"

class Bell : public Object {
public:
    Bell(int x, int y, int spawner_id);
    ~Bell();

    void step() override;

private:
    Sprite *sprite;
    SDL_FRect dst_rect;
    float x, y;
    int spawner_id;
    bool triggered = false;
    Uint64 timer = 0;
};

class BellFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x = 0, y = 0, spawner_id = 0;
        sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id);
        return new Bell(x, y, spawner_id);
    }
};

#endif
