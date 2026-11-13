#pragma once

#include <string>

#include "object/entity.h"

#define LADDER_OBJ "ladder"

class Ladder : public thoom::Entity {
 public:
  Ladder(const thoom::Entity::Context& context, int x, int y,
         std::string next_map, int which_coord);
  ~Ladder();

  void step() override;
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 private:
  float x, y;
  int which_coord;
  std::string next_map;
};
