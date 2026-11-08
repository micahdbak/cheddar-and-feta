#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "game_hud.h"
#include "hitbox.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

#define FETA_OBJ "feta"

class RemoteFeta : public IMouse, public HitSource {
 public:
  RemoteFeta(int x, int y, int animation);
  ~RemoteFeta();

  // Object
  void step() override;
  void save_data() override;

  // IMouse
  void attack(int damage) override;
  void push_item(const std::string& item_id) override;
  void push_cheese(int amount) override;
  void remove_item(const std::string& item_id) override;
  void force_dance(uint64_t timeout_ms) override;
  void set_throw(int throw_x, int throw_y) override;
  void set_max_mov_speed(float max_mov_speed) override;
  void signal_down() override {};

  // HitSource
  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }
  void hitsource_notify() override;

  void synchronize();

  // directly acted on by HostPeerReceiver
  // IMouse: x, y, is_down
  int frame_i = 0, animation = 0, which_emote = -1;
  std::vector<std::pair<std::string, std::string>> new_objects;
  bool synchronized = false;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int tile_x, tile_y;

  thoom::Sprite *sprite, *emotes;
  SDL_FRect dst_rect, emote_rect;

  std::vector<GameHud::Item> items;
};
