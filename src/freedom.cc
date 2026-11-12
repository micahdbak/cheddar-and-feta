#include "freedom.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "mouse.h"
#include "object/object.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "utils/utils.h"

Freedom::Freedom(int x, int y) : thoom::Object(nullptr) {
  this->x = x;
  this->y = y;
}

Freedom::~Freedom() {}

void Freedom::step() {
  if (cheddar == nullptr || remote_feta == nullptr) {
    return;
  }

  if (this->timer_start > 0) {
    if (thoom::Scene::instance->ticks - this->timer_start > 2000) {
      thoom::Scene::instance->request_map("maps/credits", true);
    }

    return;
  }

  float cdist =
      thoom::distance_between_points(this->x, this->y, cheddar->x, cheddar->y);
  float fdist = thoom::distance_between_points(this->x, this->y, remote_feta->x,
                                               remote_feta->y);

  // when either mouse goes 64px away from the ladder
  if (cdist > 64.0f || fdist > 64.0f) {
    // eternally dance
    cheddar->force_dance(UINT64_MAX);
    remote_feta->force_dance(UINT64_MAX);
    this->timer_start = thoom::Scene::instance->ticks;

    thoom::save.puti(GAME_DONE, 1);
    thoom::save.write_file(thoom::save.geti(SAVE_FILE));
  }
}

thoom::Object* Freedom::create_object(const std::string& options) {
  int x = 0, y = 0;
  if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new Freedom(x, y);
}
