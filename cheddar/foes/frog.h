#pragma once

#include <string>

#include "foe.h"
#include "mouse.h"
#include "object.h"
#include "scene.h"
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
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect, icon_src, icon_dst;
  Uint64 timer = 0, attack_timer = 0, hurt_timer = 0;

  int max_health = 10, health = 10;
};
