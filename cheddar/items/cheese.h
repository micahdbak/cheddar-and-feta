#ifndef ITEM_CHEESE
#define ITEM_CHEESE "item_cheese"

#include "item.h"
#include "object.h"

// dropped cheese
class Cheese : public Object {
 public:
  static std::string Options(int x, int y, int amount) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d", x, y, amount);
    return std::string(buff);
  }

  Cheese(float x, float y, int amount);
  ~Cheese();

  void step() override;

  static void drop_cheese(int x, int y, int min_amount, int max_amount);

 private:
  void render_cheese();

  std::string tex_id;
  SDL_Texture* texture = nullptr;
  SDL_FRect dst_rect;
  float x, y;
  int amount, depth_offset;
};

class CheeseFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) {
    int x, y, amount;
    int fields = sscanf(options.c_str(), "%d,%d,%d", &x, &y, &amount);

    if (fields == 2) {
      amount = 1;
    } else if (fields != 3)
      FATAL_ERROR

    return new Cheese(float(x), float(y), amount);
  }
};

#endif
