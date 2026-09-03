#include "sprite.h"

#include <iostream>

#include "bmp_texture.h"
#include "game.h"

Sprite::Sprite(const char* bmp_path, int frame_w, int frame_h, int interval_ms)
    : frame_w(frame_w), frame_h(frame_h), interval_ms(interval_ms) {
  this->texture = load_bmp_texture(bmp_path);
  this->sheet_w = this->texture->w;
  this->sheet_h = this->texture->h;

  this->frame.x = 0.0f;
  this->frame.y = 0.0f;
  this->frame.w = float(frame_w);
  this->frame.h = float(frame_h);

  this->frame_i = 0;
  this->frame_last_set = game->ticks;

  this->tex_id = bmp_path;
}

Sprite::~Sprite() { this->texture = nullptr; }

void Sprite::set_animation(int animation) {
  if (this->animation == animation) return;

  this->animation = animation;
  this->frame.y = float(this->frame_h * animation) + 0.01f;
}

bool Sprite::update_frame() {
  if (game->ticks - this->frame_last_set > this->interval_ms) {
    this->frame_i++;
    if (this->frame_i * this->frame_w >= this->sheet_w) {
      this->frame_i = 0;
    }
    this->frame.x = float(this->frame_i * this->frame_w) + 0.01f;
    this->frame_last_set = game->ticks;

    return true;
  }

  return false;
}

void Sprite::set_frame(int frame_i) {
  this->frame_i = frame_i;
  this->frame.x = float(this->frame_i * this->frame_w) + 0.1f;
  this->frame_last_set = game->ticks;
}
