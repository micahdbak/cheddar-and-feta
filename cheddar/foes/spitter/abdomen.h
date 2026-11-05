#pragma once

#include <string>

#include "hurtbox.h"
#include "object.h"
#include "scene.h"
#include "sprite.h"

#define FOE_SPITTER_ABDOMEN_OBJ "foe_spitter_abdomen"

#define FRAMES_TO_SET_CUR_DIR 4

class SpitterAbdomen : public thoom::Object, public HurtSource {
 public:
  SpitterAbdomen(int parent_id);
  ~SpitterAbdomen();

  void step() override;

  float hurtsource_x() override { return this->x; }
  float hurtsource_y() override { return this->y; }

  void attack(int damage) override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  int parent_id;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};
