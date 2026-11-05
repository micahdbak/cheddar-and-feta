#pragma once

#include <string>
#include <vector>

#include "hitbox.h"
#include "hud.h"
#include "mouse.h"
#include "object.h"
#include "scene.h"
#include "sprite.h"

#define CHEDDAR_OBJ "cheddar"

class Cheddar : public Mouse, public HitSource {
 public:
  Cheddar(std::vector<Mouse::SpawnCoord>& coordinates, std::string options);
  ~Cheddar();

  // Object
  void step() override;
  void save_data() override;

  // Mouse
  void attack(int damage) override;
  void push_item(const std::string& item_id) override;
  void push_cheese(int amount) override;
  void remove_item(const std::string& item_id) override;
  void force_dance(Uint64 timeout_ms) override;
  void set_throw(int throw_x, int throw_y) override;
  void set_max_mov_speed(float max_mov_speed) override;
  void signal_down() override;

  // HitSource
  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }
  void hitsource_notify() override { this->did_hit = true; }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int tile_x, tile_y;

  thoom::Sprite *sprite, *emotes;
  SDL_FRect dst_rect, emote_rect;
  int which_emote = -1;

  int health = 10, max_health = 10;
  std::vector<Hud::Item> items;
  int sel_item = -1;

  int throw_x = 0, throw_y = 0;
  float max_mov_speed = MOUSE_DEFAULT_SPEED;

  enum Busy {
    FALSE,
    ATTACKING,
    ATTACKED,
    THROWING,
    EATING,
    DOWNED,
    FORCED_DANCE
  } is_busy = Cheddar::Busy::FALSE;
  Uint64 busy_ticks = 0, is_down_ticks = 0, dance_until = 0;
  bool did_hit = false;
};
