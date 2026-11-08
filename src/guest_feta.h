#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "game_hud.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

#define FETA_OBJ "feta"

class GuestFeta : public thoom::Object {
 public:
  GuestFeta(float x, float y, int animation, std::vector<GameHud::Item> items);
  ~GuestFeta();

  void step() override;
  void render() override;

  void attack(int damage);
  void push_item(const std::string& item_id);
  void push_cheese(int amount);
  void remove_item(const std::string& item_id);
  void force_dance(uint64_t timeout_ms);
  void set_throw(int throw_x, int throw_y);
  void set_max_mov_speed(float max_mov_speed);

  void set_animation(int animation);
  void set_items(const std::string& items_s);

  float x = 0.0f, y = 0.0f;
  bool is_down = false, did_hit = false;
  thoom::Sprite *sprite, *emotes;
  int which_emote = -1;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  static bool known_item(const GameHud::Item& item);

  int tile_x, tile_y;

  SDL_FRect dst_rect, emote_rect;

  int health = 10, max_health = 10;
  std::vector<GameHud::Item> items;
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
  } is_busy = GuestFeta::Busy::FALSE;
  uint64_t busy_ticks = 0, is_down_ticks = 0, dance_until = 0;
};

extern GuestFeta* guest_feta;
