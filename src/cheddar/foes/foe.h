#ifndef FOE_H
#define FOE_H

#include "object.h"

#include <vector>

#define CENTER_TILE_X(_x) ((float)(int)((_x * game->tile_width) + (game->tile_width / 2)) + 0.5f)
#define CENTER_TILE_Y(_y) ((float)(int)((_y * game->tile_height) + (game->tile_height / 2)) + 0.5f)

class Mouse; // forward declaration

enum FoeStrafe {
    STRAFE_LEFT,
    NO_STRAFE,
    STRAFE_RIGHT
};

void foe_path_find(Mouse *mouse);
bool foe_move_towards(Mouse *mouse, int *x, int *y, FoeStrafe strafe);
bool foe_move_away(Mouse *mouse, int *x, int *y);
bool foe_move_circle(Mouse *mouse, int *x, int *y);
void foe_pick_random(int *x, int *y);
void foe_debug_tile(int x, int y);

class Foe : public virtual Object {
public:
    Foe(float x, float y, int spawner_id, int speed, float stalking_distance, float action_distance);
    ~Foe();

    // must be implemented per foe
    virtual void action(Mouse *mouse) = 0;
    virtual void attack_internal(int damage) = 0;

    void attack(int damage);

    // manages state and movement
    void remove_from_foes();
    void foe_step();

    enum State { IDLE, FORCE_RANDOM_TILE, WALKING, ACTION, HURT, DEAD } state;
    enum TileChoice { TOWARDS, AWAY, CIRCLE } tile_choice = TOWARDS;

    float x, y;
    float off_x = 0.0f, off_y = 0.0f;
    int direction, x_dir, y_dir;

protected:
    int icon_offset = 8;
    float current_distance;
    Mouse *target_mouse = nullptr;
    int _displayed_direction;

private:
    int spawner_id, speed;
    float stalking_distance, action_distance;
    float speed_offset;

    int target_x, target_y;
    float start_x, start_y;
    Uint64 start_ticks = 0, target_ticks = 0, direction_timer = 0;
    int walking_time, walk_offset = 0;

    FoeStrafe strafe;
};

extern std::vector<Foe *> foes;

#endif
