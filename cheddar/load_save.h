#pragma once

#include <vector>

#include "object.h"
#include "scene.h"

#define LOAD_SAVE_OBJ "load_save"

class LoadSave : public thoom::Object {
 public:
  LoadSave();
  ~LoadSave() = default;

  void step();

  std::vector<std::string> summaries;
  int sel_save = 0, save_start = 0, save_end = 0;
  bool render = true;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);
};
