#include "thorax.h"

SpitterThorax::SpitterThorax(Spitter *parent, std::shared_ptr<bool> deleted_ptr):
    parent(parent), deleted_ptr(deleted_ptr) {
    this->sprite = new Sprite("sprites/foe_spitter_thorax.bmp", 48, 48, 100);
    this->dst_rect.w = 48.0f;
    this->dst_rect.h = 48.0f;
}

SpitterThorax::~SpitterThorax() {
    delete this->sprite;
}

void SpitterThorax::step() {
    if (*this->deleted_ptr) {
        game->delete_object = true;
        return;
    }

    this->dst_rect.x = this->parent->x - 24.0f - game->corner_x;
    this->dst_rect.y = this->parent->y - 24.0f - game->corner_y;

    this->sprite->update_frame();
    this->sprite->set_animation(this->parent->displayed_direction);
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
}
