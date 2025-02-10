#ifndef ENEMY_H
#define ENEMY_H

#include "object.h"

#include <vector>

class Enemy : public Object {
public:
    Enemy();
    ~Enemy();

    void enemy_step();

    bool attack(int damage);

    float x, y;
    bool dead = false;

protected:
    // configurables; should be set in constructor
    float speed = 16.0f, sight_distance = 128.0f, attack_distance = 32.0f;
    int animation; // direction facing, from 0 to 7, counter-clockwise from south
    int height = 16; // height of sprite; used for drawing health bar

    int max_health = 2, health = 2;
    int damage = 1, armour = 0;

    int target_x, target_y; // must be set to int(x), int(y) in enemy constructor

private:
    void _draw_health_bar();
    void _draw_skull_and_bones();

    float _speed;
    Uint64 attack_ticks = 0;
    SDL_FRect draw_rect = { -999.0f, 0.0f, 0.0f, 0.0f };
};

extern std::vector<Enemy *> enemies;

#endif
