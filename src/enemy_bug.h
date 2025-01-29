#ifndef ENEMY_BUG_OBJ
#define ENEMY_BUG_OBJ "enemy.bug"

#include "enemy.h"
#include "sprite.h"

class EnemyBug : public Enemy {
public:
    EnemyBug(const std::string &options);
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
        return new EnemyBug(options);
    }
};

#endif
