#pragma once

#include "hitbox.h"
#include "item.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/thoom.h"

#define ITEM_MOLOTOV "item_molotov"

class DroppedMolotov : public DroppedItem {
 public:
  DroppedMolotov(float x, float y)
      : DroppedItem(ITEM_MOLOTOV, x, y, "sprites/item_molotov.bmp", 16, 16,
                    100) {}
  ~DroppedMolotov() = default;

  void take(Mouse* mouse) override { mouse->push_item(ITEM_MOLOTOV); }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);
};

// ----

class ThrownMolotov : public thoom::Object, public HitSource {
 public:
  ThrownMolotov(float x, float y, int x_dir, int y_dir, thoom_gid from_gid);
  ~ThrownMolotov();

  void step() override;
  void render() override;

  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }

  void hitsource_notify() override { this->did_hit = true; }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  float x, y;
  bool did_hit;
  thoom::Sprite* sprite;
  SDL_FRect dst_rect;
  int x_dir, y_dir;
  Uint64 timer;
};
