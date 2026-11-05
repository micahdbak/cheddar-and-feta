#pragma once

#include <string>

#include "foe.h"
#include "hitbox.h"
#include "mouse.h"
#include "object.h"
#include "scene.h"
#include "sprite.h"

#define FOE_BAT_OBJ "foe_bat"

class FoeBat : public Foe, public HitSource {
 public:
  FoeBat(int x, int y, int spawner_id);
  ~FoeBat();

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
  Uint64 timer = 0, hurt_timer = 0, audio_timer = 0;
  int ticks_offset = 0, base_ticks_offset = 0;

  int max_health = 4, health = 4;
};
