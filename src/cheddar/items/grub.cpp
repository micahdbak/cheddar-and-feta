#include "game.h"
#include "grub.h"

#include <map>

ThrownGrub::ThrownGrub(float x, float y, int x_dir, int y_dir, int from_id):
    TrackingItem(x, y, 24.0f, from_id), x_dir(x_dir), y_dir(y_dir) {
    this->sprite = new Sprite("sprites/item_grub_thrown.bmp", 24, 24, 50);
    this->timer = game->ticks;
}

ThrownGrub::~ThrownGrub() {
    delete this->sprite;
}

void ThrownGrub::on_mouse(Mouse *foe) {
    this->hit_nearby_foes();
}

void ThrownGrub::on_foe(Foe *foe) {
    this->hit_nearby_foes();
}

void ThrownGrub::step() {
    switch (this->state) {
    case ThrownGrub::State::FLYING:
        this->x += (float)this->x_dir * 128.0f * game->delta;
        this->y += (float)this->y_dir * 128.0f * game->delta;

        this->tracking_step();

        // first condition makes sure the above tracking_step didn't hit a mouse/foe
        if (this->state != ThrownGrub::State::EXPLODING &&
            (game->ticks - this->timer > 1000 || game->point_in_collider(this->x, this->y))) {
            this->hit_nearby_foes();
            return;
        }

        break;

    case ThrownGrub::State::EXPLODING:
        if (game->ticks - this->timer >= 1000) {
            game->delete_object = true;
            return;
        }

        break;
    }

    this->sprite->update_frame();

    this->dst_rect.w = this->sprite->frame_w;
    this->dst_rect.h = this->sprite->frame_h;
    this->dst_rect.x = this->x - (float)(int)(this->sprite->frame_w/2) - game->corner_x;
    this->dst_rect.y = this->y - (float)(int)(this->sprite->frame_h/2) - game->corner_y;

    game->push_sprite("sprites/item_grub_thrown.bmp", this->sprite->texture, &this->sprite->frame, &this->dst_rect, 8);
}

void ThrownGrub::hit_nearby_foes() {
    std::vector<std::pair<float, Foe *>> dist_foes;

    for (int i = 0; i < foes.size(); i++) {
        float dist = distance_between_points(foes[i]->x, foes[i]->y, this->x, this->y);
        dist_foes.emplace_back(dist, foes[i]);
    }

    sort(dist_foes.begin(), dist_foes.end(), [](auto &a, auto &b)-> bool {
        return a.first < b.first;
    });

    for (int num_hit = 0; num_hit < 5 && num_hit < dist_foes.size(); num_hit++) {
        dist_foes[num_hit].second->attack(5);
    }

    this->timer = game->ticks;
    this->sprite->interval_ms = 250;
    this->sprite->set_animation(1);
    this->sprite->set_frame(0);
    this->state = ThrownGrub::State::EXPLODING;
}
