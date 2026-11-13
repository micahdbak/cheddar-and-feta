#include "freedom.h"

#include <cstdint>
#include <cstdio>
#include <vector>

#include "mouse.h"
#include "object/entity.h"
#include "object/object.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "thoom/log.h"
#include "utils/utils.h"

Freedom::Freedom(int x, int y) : thoom::Object(nullptr) {
  this->x = x;
  this->y = y;
}

Freedom::~Freedom() {}

void Freedom::step() {
  if (this->timer_start > 0) {
    if (thoom::Scene::instance->ticks - this->timer_start > 2000) {
      thoom::Scene::instance->request_map("maps/credits", true);
    }

    return;
  }

  std::vector<Mouse*> mice;
  bool should_finish = false;
  const std::vector<thoom::Entity*> entities =
      thoom::Scene::instance->find_entities(
          thoom::Entity::AuthorityScope::kAll);
  for (thoom::Entity* entity : entities) {
    Mouse* mouse = dynamic_cast<Mouse*>(entity);
    if (mouse == nullptr) {
      continue;
    }

    mice.push_back(mouse);
    if (thoom::distance_between_points(this->x, this->y, mouse->x, mouse->y) >
        64.0f) {
      should_finish = true;
    }
  }

  if (!should_finish) {
    return;
  }

  for (Mouse* mouse : mice) {
    mouse->force_dance(UINT64_MAX);
  }

  this->timer_start = thoom::Scene::instance->ticks;
  thoom::save.puti(GAME_DONE, 1);
  thoom::save.write_file(thoom::save.geti(SAVE_FILE));
}

thoom::Object* Freedom::create_object(const std::string& options) {
  int x = 0, y = 0;
  if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) {
    thoom_log_fatal("%s: bad options '%s'", __func__, options.c_str());
    return nullptr;
  }

  return new Freedom(x, y);
}
