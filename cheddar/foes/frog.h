#pragma once

#include "foe.h"
#include "game.h"
#include "sprite.h"

#define FOE_FROG_OBJ "foe_frog"

class FoeFrog : public Foe {
 public:
  FoeFrog(int x, int y, int spawner_id);
  ~FoeFrog();

  void action(Mouse* mouse) override;
  void attack_internal(int damage) override;

  void step() override;

  Foe::State prev_state = Foe::State::IDLE;

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect, icon_src, icon_dst;
  Uint64 timer = 0, attack_timer = 0, hurt_timer = 0;

  int max_health = 10, health = 10;
};

class FoeFrogFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    int x = 0, y = 0, spawner_id = 0;
    if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id))
      FATAL_ERROR

    return new FoeFrog(x, y, spawner_id);
  }
};
