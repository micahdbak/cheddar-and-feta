#include "bmp_texture.h"
#include "cheese.h"
#include "game.h"
#include "mouse.h"

#include <iostream>

Cheese::Cheese(float x, float y, int amount) {
    this->x = x;
    this->y = y;
    this->amount = amount;
    this->render_cheese();
}

Cheese::~Cheese() {}

void Cheese::render_cheese() {
    char args[256];
    snprintf(args, sizeof(args), "%d", this->amount);
    this->tex_id = RENDER_CHEESE + args;
    this->texture = load_bmp_texture(this->tex_id);

    this->depth_offset = this->texture->h - 2;
    this->dst_rect.w = float(this->texture->w);
    this->dst_rect.h = float(this->texture->h);
}

void Cheese::step() {
    Mouse *mouse = closest_mouse(this->x, this->y, 8.0f, true);
    if (mouse != nullptr) {
        this->amount -= mouse->add_cheese(this->amount);
        if (this->amount == 0) {
            game->delete_object = true;
            return;
        } else {
            this->render_cheese();
        }
    }

    this->dst_rect.x = float(this->x - game->corner_x - 9);
    this->dst_rect.y = float(this->y - game->corner_y - this->depth_offset);
    game->push_sprite(this->tex_id, this->texture, NULL, &this->dst_rect, this->depth_offset);
}

void Cheese::drop_cheese(int x, int y, int min_amount, int max_amount) {
    static int consecutive_fails = 0;

    if (consecutive_fails < 4 && SDL_rand((1 + consecutive_fails) * 2) == 0) {
        consecutive_fails++;
        return;
    }

    consecutive_fails = 0;
    int amount = min_amount + ((max_amount - min_amount) > 0 ? SDL_rand(max_amount - min_amount) : 0);
    char cheese_opt[256];
    game->push_object(ITEM_CHEESE DROPPED_OBJ, Cheese::Options(x, y, amount));
}