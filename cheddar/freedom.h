#pragma once

#include "game.h"
#include "object.h"

#define FREEDOM_OBJ "freedom"

class Freedom : public thoom::Object {
 public:
  Freedom(int x, int y);
  ~Freedom();

  void step() override;

 private:
  int x, y;
  Uint64 timer_start = 0;
};

class FreedomFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) {
    int x = 0, y = 0;
    if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) FATAL_ERROR

    return new Freedom(x, y);
  }
};
