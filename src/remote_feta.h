#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

#include "hitbox.h"
#include "mouse.h"
#include "object/entity.h"

#define FETA_OBJ "feta"

class RemoteFeta : public Mouse, public HitSource {
 public:
  RemoteFeta(const thoom::Entity::Context& context, int x, int y,
             int animation);
  ~RemoteFeta();

  // Object
  void step() override;

  // Mouse
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

  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

  // directly acted on by HostPeerReceiver
  // Mouse: x, y, is_down
  bool synchronized = false;
  Uint64 last_feta_info_ticks = 0, sync_sent_ticks = 0;
};
