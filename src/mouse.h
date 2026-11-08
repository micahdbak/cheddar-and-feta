#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "game_hud.h"
#include "object.h"
#include "sprite/sprite.h"

#define MOUSE_DEFAULT_SPEED 80.0f

// save keys
#define MOUSE_X "_x"
#define MOUSE_Y "_y"
#define MOUSE_ANIMATION "_animation"
#define MOUSE_ITEMS "_items"
#define MOUSE_SPAWN_AT DONT_WRITE "spawn_at"

// animations
#define ATTACKING_ANIMATION 16
#define ATTACKED_ANIMATION 24
#define THROWING_ANIMATION 32
#define DOWN_ANIMATION 40
#define EATING_ANIMATION 48
#define DANCING_ANIMATION 49
#define DANCING2_ANIMATION 50
#define SLEEPING_ANIMATION 51

class IMouse : public thoom::Object {
 public:
  IMouse() = default;
  ~IMouse() = default;

  virtual void attack(int damage) = 0;
  virtual void push_item(const std::string& item_id) = 0;
  virtual void push_cheese(int amount) = 0;
  virtual void remove_item(const std::string& item_id) = 0;
  virtual void force_dance(uint64_t timeout_ms) = 0;
  virtual void set_throw(int throw_x, int throw_y) = 0;
  virtual void set_max_mov_speed(float max_mov_speed) = 0;
  virtual void signal_down() = 0;

  static std::string encode_items(const std::vector<GameHud::Item>& items);
  static std::vector<GameHud::Item> read_items(const std::string& items_s);
  static IMouse* closest_mouse(float x, float y, float min_distance,
                               bool forced);

  float x = 0.0f, y = 0.0f;
  bool is_down = false, is_feta = false;
};

class Mouse : public IMouse {
 public:
  struct SpawnCoord {
    float x, y;
    int animation;
  };

  enum Busy {
    FALSE,
    ATTACKING,
    ATTACKED,
    THROWING,
    EATING,
    DOWNED,
    FORCED_DANCE
  };

  Mouse(const char* sprite_path, bool is_feta);
  ~Mouse();

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

  void place(float x, float y);

  static bool check_collision(float x, float y);

  thoom::Sprite *sprite, *emotes;
  SDL_FRect dst_rect, emote_rect;
  int which_emote = -1;

  int health = 10, max_health = 10;
  std::vector<GameHud::Item> items;
  int sel_item = -1;

  int throw_x = 0, throw_y = 0;
  float max_mov_speed = MOUSE_DEFAULT_SPEED;

  Busy is_busy = Busy::FALSE;
  uint64_t busy_ticks = 0, is_down_ticks = 0, dance_until = 0;
  bool did_hit = false;

 protected:
  virtual void on_down() {}

 private:
  std::string selected_item() const;
  void move(int x_dir, int y_dir, float mov_speed);
  void draw();

  int tile_x = 0, tile_y = 0;
};

extern IMouse *cheddar, *remote_feta;
