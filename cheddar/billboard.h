#ifndef BILLBOARD_OBJ
#define BILLBOARD_OBJ "billboard"

#include <cstdio>
#include <iostream>

#include "game.h"
#include "object.h"
#include "sprite.h"

class Billboard : public Object {
 public:
  Billboard(int x, int y, int ts_x, int ts_y, int w, int h, int depth,
            std::string tilesheet);
  ~Billboard();

  void step() override;

 private:
  int x, y, depth;
  Sprite* sprite;
  SDL_FRect dst_rect;
};

class BillboardFactory : public ObjectFactory {
 public:
  static void ParseOptions(const std::string& options, int* x, int* y,
                           int* ts_x, int* ts_y, int* w, int* h, int* depth,
                           std::string& tilesheet) {
    char buff[1024];
    if (8 != sscanf(options.c_str(), "%d,%d,%d,%d,%d,%d,%d,%1023[^\n]", x, y,
                    ts_x, ts_y, w, h, depth, buff))
      FATAL_ERROR

    tilesheet = buff;
  }

  Object* create(const std::string& options) {
    int x, y, ts_x, ts_y, w, h, depth;
    std::string tilesheet;

    BillboardFactory::ParseOptions(options, &x, &y, &ts_x, &ts_y, &w, &h,
                                   &depth, tilesheet);
    return new Billboard(x, y, ts_x, ts_y, w, h, depth, tilesheet);
  }
};

#endif
