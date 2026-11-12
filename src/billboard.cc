#include "billboard.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "object/entity.h"
#include "renderer/renderer.h"
#include "sprite/sprite.h"

Billboard::Billboard(const thoom::Entity::Context& context, int x, int y,
                     int ts_x, int ts_y, int w, int h, int depth,
                     std::string tilesheet)
    : thoom::Entity(context) {
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

void Billboard::render() {
  this->dst_rect.x = (float)this->x;
  this->dst_rect.y = (float)this->y;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, this->depth);
}

void Billboard::parse_options(const std::string& options, int* x, int* y,
                              int* ts_x, int* ts_y, int* w, int* h, int* depth,
                              std::string& tilesheet) {
  char buff[1024];
  if (8 != sscanf(options.c_str(), "%d,%d,%d,%d,%d,%d,%d,%1023[^\n]", x, y,
                  ts_x, ts_y, w, h, depth, buff)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  tilesheet = buff;
}

thoom::Entity* Billboard::create_entity(const thoom::Entity::Context& context,
                                        const std::string& options) {
  int x, y, ts_x, ts_y, w, h, depth;
  std::string tilesheet;

  Billboard::parse_options(options, &x, &y, &ts_x, &ts_y, &w, &h, &depth,
                           tilesheet);
  return new Billboard(context, x, y, ts_x, ts_y, w, h, depth, tilesheet);
}
