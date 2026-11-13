#pragma once

#include <SDL3/SDL.h>

#include <cinttypes>
#include <cstdio>
#include <string>
#include <unordered_map>

#include "game_hud.h"
#include "mouse.h"
#include "object/entity.h"
#include "sprite/sprite.h"
#include "thoom/thoom.h"

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

bool require_item(const GameHud::Item& item);

class DroppedItem : public thoom::Entity {
 public:
  static std::string options(int x, int y) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d", x, y);
    return std::string(buff);
  }

  DroppedItem(const thoom::Entity::Context& context, std::string item_id,
              float x, float y, const char* sprite_path, int frame_w,
              int frame_h, int interval_ms);
  ~DroppedItem();

  void step() override;
  void render() override;

  virtual void take(Mouse* mouse) = 0;

 protected:
  std::string item_id;
  float x, y;

  thoom::Sprite* sprite;

  bool spawned_item = false;

 private:
  SDL_FRect dst_rect;
};

namespace UseItem {
static std::string options(int x, int y, int x_dir, int y_dir,
                           thoom_gid from_gid) {
  char buff[256];
  snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%" PRIu64, x, y, x_dir, y_dir,
           from_gid);
  return std::string(buff);
}
}  // namespace UseItem
