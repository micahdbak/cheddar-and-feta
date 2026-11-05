#pragma once

#include <string>

#include "object.h"
#include "scene.h"

#define FREEDOM_OBJ "freedom"

class Freedom : public thoom::Object {
 public:
  Freedom(int x, int y);
  ~Freedom();

  void step() override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  int x, y;
  Uint64 timer_start = 0;
};
