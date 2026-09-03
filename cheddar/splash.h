#ifndef SPLASH_OBJ
#define SPLASH_OBJ "splash"

#include "object.h"
#include "sprite.h"

#define NUM_ANTS 3

class Splash : public Object {
 public:
  Splash();
  ~Splash();

  void step() override;

 private:
  SDL_Texture *overlay, *tunnel;
  Sprite *sprite, *ant;
  SDL_FRect dst_rect, overlay_rect, ched_src, ched_dst, feta_src, feta_dst,
      tile_src, tile_dst, ant_src[NUM_ANTS], ant_dst[NUM_ANTS];
  Uint64 initial_timer, timer;
  int animation = 0, animations;
};

class SplashFactory : public ObjectFactory {
 public:
  Object* create(const std::string& _) { return new Splash(); }
};

#endif