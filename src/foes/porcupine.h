#pragma once

#include <string>

#include "foe.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

#define FOE_PORCUPINE_OBJ "foe_porc"

class FoePorcupine : public Foe {
 public:
  FoePorcupine(int x, int y, int spawner_id);
  ~FoePorcupine();

  void action(Mouse* mouse) override;
  void attack_internal(int damage) override;

  void step() override;

  Foe::State prev_state = Foe::State::IDLE;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect, icon_src, icon_dst;
  Uint64 timer = 0, hurt_timer = 0;

  int max_health = 12, health = 12;
  int spine_x_dir = 0, spine_y_dir = 0;
};
