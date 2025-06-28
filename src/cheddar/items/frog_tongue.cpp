#include "frog_tongue.h"
#include "items/item.h"

FrogTongue::FrogTongue(float x, float y, int x_dir, int y_dir, int from_id): TrackingItem(x, y, 48.0f, from_id), x_dir(x_dir), y_dir(y_dir) {
    int dx, dy;
    if (this->x_dir != 0 && this->y_dir != 0) {
        dx = this->x_dir * 20;
        dy = this->y_dir * 20;
    } else {
        dx = this->x_dir * 30;
        dy = this->y_dir * 30;
    }

    this->x += float(dx);
    this->y += float(dy);

    this->sprite = new Sprite("sprites/item_frog_tongue_use.bmp", 32, 32, 125);
    this->sprite->set_animation(direction_from_dirs(x_dir, y_dir));
    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;
    this->ticks = game->ticks;
}

FrogTongue::~FrogTongue() {
    delete this->sprite;
}

void FrogTongue::step() {
    if (game->ticks - this->ticks > 500) {
        game->delete_object = true;
        return;
    }

    this->tracking_step();
    this->sprite->update_frame();
    this->dst_rect.x = this->x - 16.0f - game->corner_x;
    this->dst_rect.y = this->y - 16.0f - game->corner_y;
    game->push_sprite("sprites/item_frog_tongue_use.bmp", this->sprite->texture, &this->sprite->frame, &this->dst_rect, 23);
}

void FrogTongue::on_foe(Foe *foe) {
    if (this->did_hit)
        return; // do nothing

    foe->attack(2);
    this->did_hit = true;
}

void FrogTongue::on_mouse(Mouse *mouse) {
    if (this->did_hit)
        return; // do nothing

    mouse->attack(2);
    this->did_hit = true;
    mouse->throw_x = this->x_dir;
    mouse->throw_y = this->y_dir;
}
