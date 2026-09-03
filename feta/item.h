#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <unordered_map>

#define ITEM_CANNON_BALL "item_cannon_ball"
#define ITEM_CHEESE "item_cheese"
#define ITEM_COFFEE_BEAN "item_coffee_bean"
#define ITEM_FIRE "item_fire"
#define ITEM_HERMES_BOOT "item_hermes_boot"
#define ITEM_MOLOTOV "item_molotov"
#define ITEM_SHIELD "item_shield"
#define ITEM_TOOTHPICK "item_toothpick"

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

namespace UseItem {
static std::string Options(int x, int y, int x_dir, int y_dir) {
  char buff[256];
  snprintf(buff, sizeof(buff), "%d,%d,%d,%d", x, y, x_dir, y_dir);
  return std::string(buff);
}
}  // namespace UseItem

extern std::unordered_map<std::string, Item> item_info;

#endif