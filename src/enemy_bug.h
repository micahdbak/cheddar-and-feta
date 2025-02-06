#ifndef ENEMY_BUG_OBJ
#define ENEMY_BUG_OBJ "enemy.bug"

#include "enemy.h"
#include "sprite.h"

class EnemyBug : public Enemy {
public:
    EnemyBug(int x, int y);
    ~EnemyBug();

    void step() override;

private:
    Sprite *sprite;
    SDL_FRect dst_rect;
    Uint64 dead_ticks = 0;
};

class EnemyBugFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x = 0, y = 0;
        sscanf(options.c_str(), "%d,%d", &x, &y);
        return new EnemyBug(x, y);
    }
};

#endif
