#ifndef SPRITE_H
#define SPRITE_H

#include <SDL3/SDL.h>

#include <string>

class Sprite {
 public:
  Sprite(const char* bmp_path, int frame_w, int frame_h, int interval_ms);
  ~Sprite();

  void set_animation(int animation);
  bool update_frame();  // true on frame update
  void set_frame(int frame_i);

  std::string tex_id;
  SDL_FRect frame;
  SDL_Texture* texture;
  int frame_w, frame_h, interval_ms, frame_i, animation = 0;

 private:
  int sheet_w, sheet_h;
  Uint64 frame_last_set;
};

#endif
