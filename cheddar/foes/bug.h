#pragma once

#include "foe.h"
#include "hitbox.h"
#include "scene.h"
#include "sprite.h"

#define FOE_BUG_OBJ "foe_bug"

class FoeBug : public Foe, public HitSource {
 public:
  FoeBug(int x, int y, int spawner_id);
  ~FoeBug();

  void action(Mouse* mouse) override;
  void attack_internal(int damage) override;

  void step() override;

  float hitsource_x() override { return this->x + this->off_x; }
  float hitsource_y() override { return this->y + this->off_y; }

  Foe::State prev_state = Foe::State::IDLE;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  thoom::Sprite* sprite;
  SDL_FRect dst_rect, icon_src, icon_dst;
  Uint64 timer = 0, hurt_timer = 0;

  int max_health = 4, health = 4;
};
