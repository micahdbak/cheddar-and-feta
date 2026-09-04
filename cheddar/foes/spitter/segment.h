#pragma once

#include "hitbox.h"
#include "hurtbox.h"
#include "spitter.h"
#include "sprite.h"

#define FOE_SPITTER_SEGMENT_OBJ "foe_spitter_segment"

#define FRAMES_TO_SET_CUR_DIR 4

class SpitterSegment : public thoom::Object,
                       public HurtSource,
                       public HitSource {
 public:
  SpitterSegment(Spitter* parent, int segment_id,
                 std::shared_ptr<bool> deleted_ptr);
  ~SpitterSegment();

  void step() override;

  float hurtsource_x() override { return this->x; }
  float hurtsource_y() override { return this->y; }

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }

  void attack(int) override {}

 private:
  int segment_id, cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  Spitter* parent;
  std::shared_ptr<bool> deleted_ptr;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
};

class SpitterSegmentFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) {
    int parent_id, segment_id;
    if (2 != sscanf(options.c_str(), "%d,%d", &parent_id, &segment_id))
      FATAL_ERROR

    thoom::Object* obj = thoom::game->get_object(parent_id);
    Spitter* spitter;

    if (obj != nullptr && (spitter = dynamic_cast<Spitter*>(obj)) != nullptr) {
      std::shared_ptr<bool> deleted_ptr = spitter->get_deleted_ptr();
      return new SpitterSegment(spitter, segment_id, deleted_ptr);
    } else {
      return nullptr;
    }
  }
};
