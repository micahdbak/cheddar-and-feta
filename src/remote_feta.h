#pragma once

#include <cstdint>
#include <string>

#include "hitbox.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"

#define FETA_OBJ "feta"

class RemoteFeta : public Mouse, public HitSource {
 public:
  RemoteFeta(int x, int y, int animation);
  ~RemoteFeta();

  // Object
  void step() override;

  // IMouse
  void attack(int damage) override;
  void push_item(const std::string& item_id) override;
  void push_cheese(int amount) override;
  void remove_item(const std::string& item_id) override;
  void force_dance(uint64_t timeout_ms) override;
  void set_throw(int throw_x, int throw_y) override;
  void set_max_mov_speed(float max_mov_speed) override;
  void signal_down() override {}

  // HitSource
  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }
  void hitsource_notify() override;

  void synchronize();

  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

  // directly acted on by HostPeerReceiver
  // IMouse: x, y, is_down
  bool synchronized = false;
};
