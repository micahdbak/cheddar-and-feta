#include "fire.h"

#include "audio_playback.h"

static int fire_counter = 0;

Fire::Fire(float x, float y, int x_dir, int y_dir)
    : x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
  this->sprite = new Sprite("sprites/fire.bmp", 16, 16, 100);
  this->dst_rect.w = this->sprite->frame_w;
  this->dst_rect.h = this->sprite->frame_h;
  this->timer = game->ticks;

  fire_counter++;
  this->sprite->set_animation(fire_counter % 2);
  this->sprite->set_frame(SDL_rand(4));

  this->mov_speed = SDL_randf() * 64.0f + 120.0f;
  this->throw_time = 100;

  if (!x_dir && !y_dir) {
    this->state = Fire::State::STATIONARY;
  } else {
    this->throw_time = cnf_max(cnf_abs(x_dir), cnf_abs(y_dir)) * 100;
  }

  // deals 3 damage over 3 seconds
  game->push_object(
      HITBOX_OBJ,
      HitBox::Options(this->id, -1, -8, -8, 16, 16,
                      HitBox::Properties{.damage = 1,
                                         .cooldown_ms = 1000,
                                         .delete_after_ms = 4000,
                                         .shared_cooldowns = true,
                                         .shared_id = HB_SHARED_FIRE}));
}

Fire::~Fire() { delete this->sprite; }

void Fire::step() {
  if (this->state == Fire::State::MOVING) {
    float dx = float(this->x_dir) *
               (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed *
               game->delta;
    float dy = float(this->y_dir) *
               (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * mov_speed *
               game->delta;
    float new_x = this->x + dx;
    float new_y = this->y + dy;

    if (!game->point_in_collider(new_x, this->y)) {
      this->x = new_x;
    }

    if (!game->point_in_collider(this->x, new_y)) {
      this->y = new_y;
    }

    if (game->ticks - this->timer > this->throw_time) {
      this->state = Fire::State::STATIONARY;
    }
  }

  if (game->ticks - this->timer > 3000) {
    game->delete_object = true;
    return;
  } else if (game->ticks - this->timer > 2875) {
    this->sprite->set_animation(3);
  } else if (game->ticks - this->timer > 2750) {
    this->sprite->set_animation(2);
    this->sprite->interval_ms = 1000;
  }

  this->sprite->update_frame();

  this->dst_rect.x = this->x - (float)(this->sprite->frame_w / 2);
  this->dst_rect.y = this->y - (float)(this->sprite->frame_h / 2);

  game->push_sprite(this->sprite->tex_id, this->sprite->texture,
                    &this->sprite->frame, &this->dst_rect, 12);
}
