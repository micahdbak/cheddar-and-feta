#include "game.h"
#include "molotov.h"
#include "fire.h"

#include <map>

ThrownMolotov::ThrownMolotov(float x, float y, int x_dir, int y_dir, int from_id):
    x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
    this->did_hit = false;
    this->sprite = new Sprite("sprites/item_molotov_thrown.bmp", 24, 24, 100);
    this->timer = game->ticks;

    HitBox::Properties props = {1, 1000, 1000};
    game->push_object(HITBOX_OBJ, HitBox::Options(this->id, from_id, -16, -16, 32, 32, props));
}

ThrownMolotov::~ThrownMolotov() {
    delete this->sprite;
}

void ThrownMolotov::step() {
    float dx = float(this->x_dir) * (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
    float dy = float(this->y_dir) * (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
    float new_x = this->x + dx;
    float new_y = this->y + dy;

    if (this->did_hit || game->ticks - this->timer > 1000 || game->point_in_collider(new_x, new_y)) {
        for (int i = 0; i < 8; i++) {
            int fire_x_dir, fire_y_dir;
            dirs_from_direction(i, &fire_x_dir, &fire_y_dir);
            game->push_object(ITEM_FIRE USE_OBJ, UseItem::Options(this->x, this->y, fire_x_dir, fire_y_dir, -1));
        }

        game->push_object(ITEM_FIRE USE_OBJ, UseItem::Options(this->x, this->y, 0, 0, -1));

        game->delete_object = true;
        return;
    }

    this->x = new_x;
    this->y = new_y;

    this->sprite->update_frame();

    this->dst_rect.w = this->sprite->frame_w;
    this->dst_rect.h = this->sprite->frame_h;
    this->dst_rect.x = this->x - (float)(int)(this->sprite->frame_w/2) - game->corner_x;
    this->dst_rect.y = this->y - (float)(int)(this->sprite->frame_h/2) - game->corner_y;

    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 8);
}
