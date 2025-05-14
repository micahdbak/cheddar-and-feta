#include "controller.h"
#include "game.h"
#include "spawner.h"

#include <iostream>

FoeSpawner **spawners = nullptr;
static int refcount = 0;

FoeSpawner::FoeSpawner(float x, float y, int spawner_id, const std::vector<std::vector<std::string>> &waves):
    x(x), y(y), spawner_id(spawner_id), waves(waves) {
    if (spawners == nullptr) {
        spawners = (FoeSpawner **)malloc(sizeof(FoeSpawner*) * 10); // NOLINT(bugprone-sizeof-expression)
    }

    spawners[spawner_id] = this;
    refcount++;
    this->sprite = new Sprite("sprites/spawner.bmp", 32, 32, 0);
    this->dst_rect.w = this->dst_rect.h = 32.0f;
}

FoeSpawner::~FoeSpawner() {
    delete this->sprite;
    spawners[this->spawner_id] = nullptr;

    refcount--;
    if (refcount <= 0) {
        refcount = 0;
        free(spawners);
    }
}

void FoeSpawner::step() {
    if (this->empty) {
        this->sprite->set_frame(1);
    }

    this->dst_rect.x = this->x - 16.0f - game->corner_x;
    this->dst_rect.y = this->y - 16.0f - game->corner_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 16.0f);

    if (local_controller.c == 'q') {
        this->trigger();
        local_controller.c = NO_CHAR;
    }
}

void FoeSpawner::trigger() {
    this->foerefs = 0;

    if (this->wave >= this->waves.size()) {
        this->empty = true;
        return;
    }

    for (std::string obj : this->waves[this->wave]) {
        char buff[256];
        snprintf(buff, sizeof(buff), "%d,%d,%d", (int)this->x, (int)this->y, this->spawner_id);
        game->push_object(obj, buff);
        this->foerefs++;
    }

    this->wave++;
}

void FoeSpawner::log_death() {
    this->foerefs--;

    if (this->foerefs <= 0) {
        this->trigger();
    }
}
