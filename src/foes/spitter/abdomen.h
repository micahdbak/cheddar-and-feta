#pragma once

#include <string>

#include "hurtbox.h"
#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/thoom.h"

#define FOE_SPITTER_ABDOMEN_OBJ "foe_spitter_abdomen"

#define FRAMES_TO_SET_CUR_DIR 4

class SpitterAbdomen : public thoom::Object, public HurtSource {
 public:
  SpitterAbdomen(thoom_gid parent_gid);
  ~SpitterAbdomen();

  void step() override;
  void render() override;

  float hurtsource_x() override { return this->x; }
  float hurtsource_y() override { return this->y; }

  void attack(int damage) override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  thoom_gid parent_gid;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
