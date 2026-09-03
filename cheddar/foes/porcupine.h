#ifndef FOE_PORCUPINE_OBJ
#define FOE_PORCUPINE_OBJ "foe_porc"

#include "foe.h"
#include "game.h"
#include "sprite.h"

class FoePorcupine : public Foe {
 public:
  FoePorcupine(int x, int y, int spawner_id);
  ~FoePorcupine();

  void action(Mouse* mouse) override;
  void attack_internal(int damage) override;

  void step() override;

  Foe::State prev_state = Foe::State::IDLE;

 private:
  Sprite* sprite;
  SDL_FRect dst_rect, icon_src, icon_dst;
  Uint64 timer = 0, hurt_timer = 0;

  int max_health = 12, health = 12;
  int spine_x_dir = 0, spine_y_dir = 0;
};

class FoePorcupineFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) override {
    int x = 0, y = 0, spawner_id = 0;
    if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id))
      FATAL_ERROR

    return new FoePorcupine(x, y, spawner_id);
  }
};

#endif
