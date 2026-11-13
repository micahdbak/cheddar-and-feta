#include "segment.h"

#include <optional>

#include "foes/spitter/spitter.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "object/entity.h"
#include "renderer/renderer.h"
#include "sprite/sprite.h"
#include "thoom/macros.h"
#include "utils/utils.h"

SpitterSegment::SpitterSegment(Spitter& spitter, int segment_id)
    : thoom::Entity::Node(spitter, std::nullopt),
      spitter(spitter),
      segment_id(segment_id) {
  this->sprite = new thoom::Sprite("sprites/foe_spitter_segment.bmp", 32, 32,
                                   50 + (segment_id * 50));
  this->sprite->set_frame((segment_id * 2) % 4);
  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;

  new HurtBox(*this, -12, -12, 24, 24);
  new HitBox(*this, nullptr, -8, -8, 16, 16,
             HitBox::Properties{.damage = 2,
                                .cooldown_ms = 1000,
                                .shared_cooldowns = true,
                                .shared_id = HB_SHARED_SPITTER});

  this->cur_dir = 0;
  for (int i = 0; i < FRAMES_TO_SET_CUR_DIR; i++) this->dirs[i] = 0;
  this->dirs_i = 0;
}

SpitterSegment::~SpitterSegment() { delete this->sprite; }

void SpitterSegment::step() {
  Spitter::step_t p1 =
      this->spitter.get_pos(THOOM_CLAMP(this->segment_id + 1, 0, NUM_SEGMENTS));
  Spitter::step_t p2 = this->spitter.get_pos(this->segment_id);
  Spitter::step_t p3 =
      this->spitter.get_pos(THOOM_CLAMP(this->segment_id - 1, 0, NUM_SEGMENTS));
  Spitter::step_t p4 =
      this->spitter.get_pos(THOOM_CLAMP(this->segment_id - 2, 0, NUM_SEGMENTS));

  if (p1.x == 0 || p2.x == 0 || p3.x == 0 || p4.x == 0) return;

  const int this_x = (p1.x + p3.x) >> 1;
  const int this_y = (p1.y + p3.y) >> 1;
  const int next_x = (p2.x + p4.x) >> 1;
  const int next_y = (p2.y + p4.y) >> 1;

  this->x = (float)this_x;
  this->y = (float)this_y;

  int x_dir, y_dir;
  thoom::dir_to_point(this_x, this_y, next_x, next_y, &x_dir, &y_dir);
  int direction = thoom::direction_from_dirs(x_dir, y_dir);

  this->dirs[this->dirs_i] = direction;
  this->dirs_i = (++this->dirs_i) % FRAMES_TO_SET_CUR_DIR;
  int sum = 0;

  for (int i = 1; i < FRAMES_TO_SET_CUR_DIR; i++) {
    sum += this->dirs[i - 1] == this->dirs[i] ? 1 : 0;
  }

  if (sum >= FRAMES_TO_SET_CUR_DIR - 1) {
    this->cur_dir = this->dirs[0];
  }

  this->sprite->set_animation(this->cur_dir);
  this->sprite->update_frame();
}

void SpitterSegment::render() {
  this->dst_rect.x = this->x - 16.0f;
  this->dst_rect.y = this->y - 16.0f;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 16);
}
