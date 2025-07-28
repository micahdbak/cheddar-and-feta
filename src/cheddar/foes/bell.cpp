#include "bell.h"
#include "controller.h"
#include "mouse.h"
#include "spawner.h"

Bell::Bell(int x, int y, int spawner_id) {
    this->x = x;
    this->y = y;
    this->spawner_id = spawner_id;
    this->sprite = new Sprite("sprites/bell.bmp", 24, 24, 50);
    this->dst_rect.w = 24.0f;
    this->dst_rect.h = 24.0f;

    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -8, -8, 16, 16));
}

Bell::~Bell() {
    delete this->sprite;
}

void Bell::attack(int damage) {
    FoeSpawner *spawner = spawners[this->spawner_id];

    if (spawner != nullptr && !spawner->empty && !this->triggered) {
        spawners[this->spawner_id]->trigger();
        this->triggered = true;
        this->sprite->set_animation(1);
    }

    this->timer = game->ticks;
}

void Bell::step() {
    if (this->triggered && game->ticks - this->timer < 1500) {
        this->sprite->update_frame();
    } else {
        this->sprite->set_frame(0);
    }

    this->dst_rect.x = this->x - 12.0f - game->corner_x;
    this->dst_rect.y = this->y - 12.0f - game->corner_y;
    game->push_sprite("sprites/bell.bmp", this->sprite->texture, &this->sprite->frame, &this->dst_rect, 16);
}
