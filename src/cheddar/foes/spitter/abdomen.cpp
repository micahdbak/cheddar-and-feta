#include "abdomen.h"

SpitterAbdomen::SpitterAbdomen(Spitter *parent, std::shared_ptr<bool> deleted_ptr):
    parent(parent), deleted_ptr(deleted_ptr) {
    this->sprite = new Sprite("sprites/foe_spitter_abdomen.bmp", 32, 32, 250);
    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;
    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -16, -16, 32, 32));

    this->cur_dir = 0;
    for (int i = 0; i < FRAMES_TO_SET_CUR_DIR; i++)
        this->dirs[i] = 0;
    this->dirs_i = 0;
}

SpitterAbdomen::~SpitterAbdomen() {
    delete this->sprite;
}

void SpitterAbdomen::step() {
    if (*this->deleted_ptr) {
        game->delete_object = true;
        return;
    }

    struct Spitter::step_t p2, p3, p4;
    int direction;

    // p1, not used
    p2 = this->parent->get_pos(NUM_SEGMENTS);
    p3 = this->parent->get_pos(NUM_SEGMENTS - 1);
    p4 = this->parent->get_pos(NUM_SEGMENTS - 2);

    if (p2.x == 0 || p3.x == 0 || p4.x == 0)
        return;

    int this_x = p2.x;
    int this_y = p2.y;
    int next_x = (p2.x + p4.x) >> 1;
    int next_y = (p2.y + p4.y) >> 1;

    this->x = (float)this_x;
    this->y = (float)this_y;

    int x_dir, y_dir;
    dir_to_point(this_x, this_y, next_x, next_y, &x_dir, &y_dir);
    direction = direction_from_dirs(x_dir, y_dir);

    this->dirs[this->dirs_i] = direction;
    this->dirs_i = (++this->dirs_i) % FRAMES_TO_SET_CUR_DIR;
    int sum = 0;

    for (int i = 1; i < FRAMES_TO_SET_CUR_DIR; i++)
        sum += this->dirs[i-1] == this->dirs[i] ? 1 : 0;

    if (sum >= FRAMES_TO_SET_CUR_DIR - 1) {
        this->cur_dir = this->dirs[0];
    }

    Mouse *mouse = Mouse::closest_mouse(this->x, this->y, 64.0f, true);

    if (mouse == nullptr) {
        this->parent->abdomen_distance = 256.0f;
    } else {
        this->parent->abdomen_distance = distance_between_points(mouse->x, mouse->y, this->x, this->y);
    }

    this->dst_rect.x = this->x - 16.0f;
    this->dst_rect.y = this->y - 16.0f;

    this->sprite->set_animation(this->cur_dir);
    this->sprite->update_frame();
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 16);
}
