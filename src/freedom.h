#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "object/object.h"

#define FREEDOM_OBJ "freedom"

class Freedom : public thoom::Object {
 public:
  Freedom(int x, int y);
  ~Freedom();

  void step() override;
  static thoom::Object* create_object(const std::string& options);

 private:
  int x, y;
  Uint64 timer_start = 0;
};
