#include "segment.h"

SpitterSegment::SpitterSegment(Spitter *parent, int segment_id, std::shared_ptr<bool> deleted_ptr):
    parent(parent), deleted_ptr(deleted_ptr) {
    this->sprite = new Sprite("sprites/foe_spitter_segment.bmp", 32, 32, 50 + (segment_id * 50));
    this->sprite->set_frame((segment_id * 2) % 4);
    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;
    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -12, -12, 24, 24));
    game->push_object(HITBOX_OBJ, HitBox::Options(this->id, -1, -8, -8, 16, 16, HitBox::Properties{
        .damage = 2,
        .cooldown_ms = 1000,
        .shared_cooldowns = true,
        .shared_id = HB_SHARED_SPITTER
    }));
    this->segment_id = segment_id;

    this->cur_dir = 0;
    for (int i = 0; i < FRAMES_TO_SET_CUR_DIR; i++)
        this->dirs[i] = 0;
    this->dirs_i = 0;
}

SpitterSegment::~SpitterSegment() {
    delete this->sprite;
}

void SpitterSegment::step() {
    if (*this->deleted_ptr) {
        game->delete_object = true;
        return;
    }

    struct Spitter::step_t p1, p2, p3, p4;
    int direction;

    p1 = this->parent->get_pos(cnf_clamp(this->segment_id + 1, 0, NUM_SEGMENTS));
    p2 = this->parent->get_pos(this->segment_id);
    p3 = this->parent->get_pos(cnf_clamp(this->segment_id - 1, 0, NUM_SEGMENTS));
    p4 = this->parent->get_pos(cnf_clamp(this->segment_id - 2, 0, NUM_SEGMENTS));

    if (p1.x == 0 || p2.x == 0 || p3.x == 0 || p4.x == 0)
        return;

    int this_x = (p1.x + p3.x) >> 1;
    int this_y = (p1.y + p3.y) >> 1;
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

    this->dst_rect.x = this->x - 16.0f - game->corner_x;
    this->dst_rect.y = this->y - 16.0f - game->corner_y;

    this->sprite->set_animation(this->cur_dir);
    this->sprite->update_frame();
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 16);
}
