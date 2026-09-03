#ifndef FETA_OBJ
#define FETA_OBJ "feta"

#include <string>
#include <vector>

#include "game.h"
#include "object.h"
#include "save_data.h"
#include "sprite.h"

#define MOUSE_DEFAULT_SPEED 80.0f

// animations
#define RUNPREP_ANIMATION 8
#define ATTACKING_ANIMATION 16
#define ATTACKED_ANIMATION 24
#define THROWING_ANIMATION 32
#define DOWN_ANIMATION 40
#define EATING_ANIMATION 48
#define DANCING_ANIMATION 49
#define DANCING2_ANIMATION 50
#define SLEEPING_ANIMATION 51

namespace Mouse {
bool check_collision(float x, float y);
std::string encode_items(const std::vector<Game::HudItem>& items);
std::vector<Game::HudItem> read_items(const std::string& items_s);
std::string audio_msg(const char* wav_path, float gain, int x, int y);
};  // namespace Mouse

class Feta : public Object {
 public:
  Feta(float x, float y, int animation, std::vector<Game::HudItem> items);
  ~Feta();

  void step() override;

  void attack(int damage);
  void push_item(const std::string& item_id);
  void push_cheese(int amount);
  void remove_item(const std::string& item_id);
  void force_dance(Uint64 timeout_ms);
  void set_throw(int throw_x, int throw_y);
  void set_max_mov_speed(float max_mov_speed);

  void set_animation(int animation);
  void set_items(const std::string& items_s);

  float x, y;
  bool is_down = false, did_hit = false;
  bool mice_locked = false;
  Sprite *sprite, *emotes;
  int which_emote = -1;

 private:
  int tile_x, tile_y;

  SDL_FRect dst_rect, emote_rect;

  int health = 10, max_health = 10;
  std::vector<Game::HudItem> items;
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
  } is_busy = Feta::Busy::FALSE;
  Uint64 busy_ticks = 0, is_down_ticks = 0, dance_until = 0;
};

class FetaFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) override {
    int animation;
    char x_str[256], y_str[256], items_s[1024];
    int fields = sscanf(options.c_str(), "%255[^,],%255[^,],%d %1023s", x_str,
                        y_str, &animation, items_s);

    if (fields == 3) {
      items_s[0] = '\0';
    } else if (fields != 4)
      FATAL_ERROR

    return new Feta(str_to_float(x_str), str_to_float(y_str), animation,
                    Mouse::read_items(items_s));
  }
};

extern Feta* feta;

#endif