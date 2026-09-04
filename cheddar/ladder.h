#pragma once

#include <iostream>

#include "game.h"
#include "object.h"
#include "sprite.h"

#define LADDER_OBJ "ladder"

class Ladder : public thoom::Object {
 public:
  Ladder(int x, int y, std::string next_map, int which_coord);
  ~Ladder();

  void step() override;

 private:
  float x, y;
  int which_coord;
  std::string next_map;
};

class LadderFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    int x, y, which_coord;
    if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &which_coord))
      FATAL_ERROR

    const char* map = options.c_str();
    while (*map != '\0' && *map != ' ') map++;

    if (*map == ' ') map++;  // don't want the last space

    if (*map == '\0') {
      // empty map
      std::cerr << "LadderFactory::create: no map provided" << std::endl;
      std::exit(1);
    }

    return new Ladder(x, y, std::string(map), which_coord);
  }
};
