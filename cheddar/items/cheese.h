#pragma once

#include "item.h"
#include "object.h"
#include "scene.h"

#define ITEM_CHEESE "item_cheese"

// dropped cheese
class Cheese : public thoom::Object {
 public:
  static std::string options(int x, int y, int amount) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d", x, y, amount);
    return std::string(buff);
  }

  Cheese(float x, float y, int amount);
  ~Cheese();

  void step() override;

  static void drop_cheese(int x, int y, int min_amount, int max_amount);
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  void render_cheese();

  std::string tex_id;
  SDL_Texture* texture = nullptr;
  SDL_FRect dst_rect;
  float x, y;
  int amount, depth_offset;
};
