#pragma once

#include <SDL3/SDL.h>

#include <cstddef>
#include <string>

#include "foes/foe.h"
#include "hitbox.h"
#include "mouse.h"
#include "object/entity.h"
#include "sprite/sprite.h"

#define FOE_SPITTER_OBJ "foe_spitter"

#define NUM_SEGMENTS 8
#define SEGMENT_DISTANCE 11

class Spitter : public Foe {
 public:
  struct step_t {
    int x, y, direction;
  };

  Spitter(const thoom::Entity::Context& context, float x, float y,
          int spawner_id);
  ~Spitter();

  void action(Mouse* mouse) override;
  void attack_internal(int damage) override;
  void receive_hit(const Hit& hit) override;

  void step() override;
  void render() override;

  struct step_t get_pos(int segment_id) {
    return this->get_step(this->steps_size - (segment_id * SEGMENT_DISTANCE));
  }

  int displayed_direction = 0;
  float abdomen_distance = 256.0f;

  Foe::State prev_state = Foe::State::IDLE;
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;

  Uint64 timer, hurt_timer, direction_timer = 0, start_timer;
  int last_direction = 0;
  int max_health = 16, health = 16;
  SDL_FRect icon_src, icon_dst;

  // circular queue
  static const size_t steps_size = NUM_SEGMENTS * SEGMENT_DISTANCE;
  size_t steps_front = 0, steps_count = 0;
  struct step_t steps[steps_size];

  void push_step(int x, int y, int direction) {
    struct step_t pos = {x, y, direction};

    if (this->steps_count >= Spitter::steps_size) {
      steps_front = (steps_front + 1) % Spitter::steps_size;
    } else {
      ++this->steps_count;
    }

    int idx = (this->steps_front + this->steps_count - 1) % Spitter::steps_size;

    this->steps[idx] = pos;
  }

  struct step_t get_step(int idx) {
    if (idx < 0) {
      return {0, 0, 0};
    }

    if (idx >= this->steps_count) {
      idx = this->steps_count - 1;
    }

    return this->steps[(this->steps_front + idx) % Spitter::steps_size];
  }
};
