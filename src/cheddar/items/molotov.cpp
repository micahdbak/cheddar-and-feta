#include "game.h"
#include "molotov.h"
#include "fire.h"

#include <map>

ThrownMolotov::ThrownMolotov(float x, float y, int x_dir, int y_dir, int from_id):
    TrackingItem(x, y, 24.0f, from_id), x_dir(x_dir), y_dir(y_dir) {
    this->sprite = new Sprite("sprites/item_molotov_thrown.bmp", 24, 24, 100);
    this->timer = game->ticks;
}

ThrownMolotov::~ThrownMolotov() {
    delete this->sprite;
}

void ThrownMolotov::on_mouse(Mouse *foe) {
    this->spawn_fire();
}

void ThrownMolotov::on_foe(Foe *foe) {
    this->spawn_fire();
}

void ThrownMolotov::step() {
    switch (this->state) {
    case ThrownMolotov::State::FLYING: {
        float dx = float(this->x_dir) * (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
        float dy = float(this->y_dir) * (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
        float new_x = this->x + dx;
        float new_y = this->y + dy;

        this->tracking_step();

        // first condition makes sure the above tracking_step didn't hit a mouse/foe
        if (this->state != ThrownMolotov::State::DELETE_OBJ &&
            (game->ticks - this->timer > 1000 || game->point_in_collider(new_x, new_y))) {
            this->spawn_fire();
            return;
        }

        if (this->state == ThrownMolotov::State::DELETE_OBJ) {
            game->delete_object = true;
            return;
        }

        this->x = new_x;
        this->y = new_y;
    } break;

    case ThrownMolotov::State::DELETE_OBJ:
        game->delete_object = true;
        return;

        break;
    }

    this->sprite->update_frame();

    this->dst_rect.w = this->sprite->frame_w;
    this->dst_rect.h = this->sprite->frame_h;
    this->dst_rect.x = this->x - (float)(int)(this->sprite->frame_w/2) - game->corner_x;
    this->dst_rect.y = this->y - (float)(int)(this->sprite->frame_h/2) - game->corner_y;

    game->push_sprite("sprites/item_molotov_thrown.bmp", this->sprite->texture, &this->sprite->frame, &this->dst_rect, 8);
}

void ThrownMolotov::spawn_fire() {
    char buff[256];

    for (int i = 0; i < 8; i++) {
        int fire_x_dir, fire_y_dir;
        dirs_from_direction(i, &fire_x_dir, &fire_y_dir);
        snprintf(buff, sizeof(buff), "%d,%d,%d,%d,0", (int)this->x, (int)this->y, fire_x_dir, fire_y_dir);
        game->push_object(ITEM_FIRE USE_OBJ, buff);
    }

    snprintf(buff, sizeof(buff), "%d,%d,0,0,0", (int)this->x, (int)this->y);
    game->push_object(ITEM_FIRE USE_OBJ, buff);

    this->state = ThrownMolotov::State::DELETE_OBJ;
}
