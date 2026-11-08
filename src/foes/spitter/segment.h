#pragma once

#include <string>

#include "hitbox.h"
#include "hurtbox.h"
#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/thoom.h"

#define FOE_SPITTER_SEGMENT_OBJ "foe_spitter_segment"

#define FRAMES_TO_SET_CUR_DIR 4

class SpitterSegment : public thoom::Object,
                       public HurtSource,
                       public HitSource {
 public:
  SpitterSegment(thoom_gid parent_gid, int segment_id);
  ~SpitterSegment();

  void step() override;

  float hurtsource_x() override { return this->x; }
  float hurtsource_y() override { return this->y; }

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }

  void attack(int) override {}
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int segment_id, cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  thoom_gid parent_gid;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
