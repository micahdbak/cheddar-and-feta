#pragma once

#include <cstdio>
#include <string>

#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

#define TOSSED_ITEM_OBJ "tossed_item"

class TossedItem : public thoom::Object {
 public:
  static std::string options(int x, int y, int x_dir, int y_dir, bool from_feta,
                             std::string item_id) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d %s", x, y, x_dir, y_dir,
             from_feta == true ? 1 : 0, item_id.c_str());
    return std::string(buff);
  }

  TossedItem(float x, float y, int x_dir, int y_dir, bool from_feta,
             std::string item_id);
  ~TossedItem();

  void step() override;
  void render() override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  float x, y;
  int x_dir, y_dir;
  bool from_feta;
  std::string item_id;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
  Uint64 spawned_ticks;
};
