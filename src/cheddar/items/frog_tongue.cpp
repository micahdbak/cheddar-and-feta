#include "frog_tongue.h"
#include "items/item.h"

FrogTongue::FrogTongue(float x, float y, int x_dir, int y_dir, int from_id):
    x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
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

    HitBox::Properties props = {2, 750, 500};
    props.single_use = true;
    game->push_object(HITBOX_OBJ, HitBox::Options(this->id, from_id, -16, -16, 32, 32, props));
}

FrogTongue::~FrogTongue() {
    delete this->sprite;
}

void FrogTongue::step() {
    if (game->ticks - this->ticks > 500) {
        game->delete_object = true;
        return;
    }

    this->sprite->update_frame();
    this->dst_rect.x = this->x - 16.0f - game->corner_x;
    this->dst_rect.y = this->y - 16.0f - game->corner_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 23);
}
