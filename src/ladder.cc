#include "ladder.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>

#include "audio/audio.h"
#include "mouse.h"
#include "object.h"
#include "save/save_data.h"
#include "scene/scene.h"

Ladder::Ladder(int x, int y, std::string next_map, int which_coord) {
  this->x = (float)x;
  this->y = (float)y;

  this->which_coord = which_coord;

  this->next_map = next_map;
}

Ladder::~Ladder() {
  // pass
}

void Ladder::step() {
  IMouse* mouse = IMouse::closest_mouse(this->x, this->y, 8, true);

  if (mouse != nullptr) {
    thoom::Scene::instance->request_map(this->next_map);
    thoom::save.puti(MOUSE_SPAWN_AT, this->which_coord);
    thoom::Audio::instance->play("sfx/ladder.wav", 1.0f, this->x, this->y,
                                 false);
  }
}

thoom::Object* Ladder::create_object(thoom::Scene* scene,
                                     const std::string& options) {
  int x, y, which_coord;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &which_coord)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  const char* map = options.c_str();
  while (*map != '\0' && *map != ' ') map++;

  if (*map == ' ') map++;  // don't want the last space

  if (*map == '\0') {
    // empty map
    std::cerr << "Ladder::create_object: no map provided" << std::endl;
    std::exit(1);
  }

  return new Ladder(x, y, std::string(map), which_coord);
}
