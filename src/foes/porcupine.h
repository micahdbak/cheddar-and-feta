#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "foe.h"
#include "mouse.h"
#include "object/entity.h"
#include "sprite/sprite.h"

#define FOE_PORCUPINE_OBJ "foe_porc"

class FoePorcupine : public Foe {
 public:
  FoePorcupine(const thoom::Entity::Context& context, int x, int y,
               int spawner_id);
  ~FoePorcupine();

  void action(Mouse* mouse) override;
  void attack_internal(int damage) override;

  void step() override;
  void render() override;

  Foe::State prev_state = Foe::State::IDLE;
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect, icon_src, icon_dst;
  Uint64 timer = 0, hurt_timer = 0;

  int max_health = 12, health = 12;
  int spine_x_dir = 0, spine_y_dir = 0;
};
