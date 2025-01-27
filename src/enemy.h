#ifndef ENEMY_H
#define ENEMY_H

#include "object.h"

class Enemy : public Object {
public:
    Enemy();
    ~Enemy();

    void enemy_step();

    bool attack();

    float x, y;

protected:
    // configurables; should be set in constructor
    float speed = 16.0f, sight_distance = 128.0f, attack_distance = 32.0f;
    int animation; // direction facing, from 0 to 7, counter-clockwise from south
    int select_w = 16, select_h = 16;

    int target_x, target_y; // must be set to int(x), int(y) in enemy constructor

private:
    float _speed;
    Uint64 attack_ticks = 0;
};

extern std::vector<Enemy *> enemies;

#endif
