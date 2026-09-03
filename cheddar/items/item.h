#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <unordered_map>

#include "../foes/foe.h"
#include "game.h"
#include "mouse.h"
#include "object.h"
#include "sprite.h"

#define ITEM_NONE "item_none"
#define DROPPED_OBJ "_dropped"
#define USE_OBJ "_use"

enum ItemType { USEFUL, THROWABLE, EDIBLE, HELD_EFFECT };

struct Item {
  ItemType type = HELD_EFFECT;
  std::string name = "Unknown Item";
  int damage = 1;
  int armour = 0;
  float speed = 1.0f;
};

extern std::unordered_map<std::string, Item> item_info;

class DroppedItem : public virtual Object {
 public:
  static std::string Options(int x, int y) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d", x, y);
    return std::string(buff);
  }

  DroppedItem(std::string item_id, float x, float y, const char* sprite_path,
              int frame_w, int frame_h, int interval_ms);
  ~DroppedItem();

  void dropped_step();

  virtual void take(Mouse* mouse) = 0;

 protected:
  std::string item_id;
  float x, y;

  Sprite* sprite;

  bool spawned_item = false;

 private:
  SDL_FRect dst_rect;
};

namespace UseItem {
static std::string Options(int x, int y, int x_dir, int y_dir, int from_id) {
  char buff[256];
  snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d", x, y, x_dir, y_dir, from_id);
  return std::string(buff);
}
}  // namespace UseItem

#endif
