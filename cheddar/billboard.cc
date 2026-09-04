#include "billboard.h"

#include "game.h"

Billboard::Billboard(int x, int y, int ts_x, int ts_y, int w, int h, int depth,
                     std::string tilesheet) {
  this->sprite = new thoom::Sprite(tilesheet.c_str(), 16, 16, 1000);
  this->sprite->set_animation(ts_y);
  this->sprite->set_frame(ts_x);
  // overwrite the frame
  this->sprite->frame.w = float(w);
  this->sprite->frame.h = float(h);

  this->x = x;
  this->y = y;
  this->dst_rect.w = w;
  this->dst_rect.h = h;

  this->depth = depth;
}

Billboard::~Billboard() { delete this->sprite; }

void Billboard::step() {
  this->dst_rect.x = (float)this->x;
  this->dst_rect.y = (float)this->y;
  thoom::game->push_sprite(this->sprite->tex_id, this->sprite->texture,
                           &this->sprite->frame, &this->dst_rect, this->depth);
}
