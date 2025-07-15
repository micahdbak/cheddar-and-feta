#include "spine.h"

ThrownSpine::ThrownSpine(float x, float y, int x_dir, int y_dir, int from_id): TrackingItem(x, y, 16.0f, from_id), x_dir(x_dir), y_dir(y_dir) {
    this->sprite = new Sprite("sprites/item_toothpick.bmp", 16, 16, 125);
    this->sprite->set_animation(direction_from_dirs(x_dir, y_dir));
    this->dst_rect.w = 16.0f;
    this->dst_rect.h = 16.0f;
    this->spawned_ticks = game->ticks;
}

ThrownSpine::~ThrownSpine() {
    delete this->sprite;
}

void ThrownSpine::step() {
    this->tracking_step();

    float dx = float(this->x_dir) * (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
    float dy = float(this->y_dir) * (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
    float new_x = this->x + dx;
    float new_y = this->y + dy;

    if (this->did_hit || game->point_in_collider(new_x, new_y) || game->ticks - this->spawned_ticks > 1000) {
        game->delete_object = true;
        return;
    }

    this->x = new_x;
    this->y = new_y;

    this->sprite->update_frame();
    this->dst_rect.x = this->x - 8.0f - game->corner_x;
    this->dst_rect.y = this->y - 8.0f - game->corner_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 23);
}
