#pragma once

#include <string>

#include "object.h"
#include "scene/scene.h"

#define LADDER_OBJ "ladder"

class Ladder : public thoom::Object {
 public:
  Ladder(int x, int y, std::string next_map, int which_coord);
  ~Ladder();

  void step() override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  float x, y;
  int which_coord;
  std::string next_map;
};
