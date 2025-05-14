#ifndef FOE_H
#define FOE_H

#include "object.h"

#include <vector>

class Mouse; // forward declaration

void foe_path_find(Mouse *mouse);
bool foe_move_towards(Mouse *mouse, int *x, int *y);
void foe_pick_random(int *x, int *y);

class Foe : public Object {
public:
    Foe(float x, float y, int spawner_id, int speed, float stalking_distance, float action_distance);
    ~Foe();

    // must be implemented per foe
    virtual void action(Mouse *mouse) = 0;
    virtual void attack(int damage) = 0;

    // manages state and movement
    void remove_from_foes();
    void foe_step();

    enum State { IDLE, WALKING, ACTION, HURT, DEAD } state;

    float x, y;
    int direction, x_dir, y_dir;
private:
    int spawner_id, speed;
    float stalking_distance, action_distance;
    float speed_offset;

    int target_x, target_y;
    float target_x_f, target_y_f;
};

extern std::vector<Foe *> foes;

#endif
