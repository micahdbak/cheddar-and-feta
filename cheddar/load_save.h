#ifndef LOAD_SAVE_OBJ
#define LOAD_SAVE_OBJ "load_save"

#include <vector>

#include "object.h"

class LoadSave : public Object {
 public:
  LoadSave();
  ~LoadSave() = default;

  void step();

  std::vector<std::string> summaries;
  int sel_save = 0, save_start = 0, save_end = 0;
  bool render = true;
};

class LoadSaveFactory : public ObjectFactory {
 public:
  LoadSaveFactory() = default;
  ~LoadSaveFactory() = default;

  Object* create(const std::string& options) override { return new LoadSave(); }
};

#endif