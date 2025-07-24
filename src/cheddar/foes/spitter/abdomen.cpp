#include "abdomen.h"

SpitterAbdomen::SpitterAbdomen(Spitter *parent, std::shared_ptr<bool> deleted_ptr):
    parent(parent), deleted_ptr(deleted_ptr) {
    this->sprite = new Sprite("sprites/foe_spitter_abdomen.bmp", 32, 32, 0);
    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;
}

SpitterAbdomen::~SpitterAbdomen() {
    delete this->sprite;
}

void SpitterAbdomen::step() {
    if (*this->deleted_ptr) {
        game->delete_object = true;
        return;
    }

    int x_dir, y_dir;
    dirs_from_direction(this->parent->displayed_direction, &x_dir, &y_dir);

    int distance = (int)(x_dir != 0 && y_dir != 0 ? 12.0f * DIAG_MULTIPLIER : 12.0f);
    float dx = (float)(x_dir * distance);
    float dy = (float)(y_dir * distance);

    this->dst_rect.x = this->parent->x - dx - 16.0f - game->corner_x;
    this->dst_rect.y = this->parent->y - dy - 16.0f - game->corner_y;

    this->sprite->set_animation(this->parent->displayed_direction);
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 14);
}
