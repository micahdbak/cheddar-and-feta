#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

#include "hud/hud.h"
#include "mouse.h"
#include "object.h"
#include "save/save_data.h"
#include "scene/scene.h"

#define ITEM_SAVE "item_save"

class ItemSaveUse : public thoom::Object {
 public:
  ItemSaveUse(bool is_feta) : is_feta(is_feta) {}
  ~ItemSaveUse() = default;

  void step() {
    // feta shouldnt trigger saves - theyre a remote player, seems insecure
    if (this->is_feta) {
      this->deleted = true;
      return;
    }

    if (cheddar != nullptr) cheddar->push_item(ITEM_SAVE);

    thoom::Scene::instance->save_objects();
    thoom::Hud::instance->display_notification(
        "Saved to file " + std::to_string(thoom::save.geti(SAVE_FILE) + 1) +
        ".");
    thoom::save.write_file(thoom::save.geti(SAVE_FILE));

    // all done
    this->deleted = true;
  }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options) {
    int obj_id;
    if (1 != sscanf(options.c_str(), "%*d,%*d,%*d,%*d,%d", &obj_id)) {
      std::exit(1);
    }

    bool is_feta = feta != nullptr && feta->id == obj_id;

    return new ItemSaveUse(is_feta);
  }

 private:
  bool is_feta;
};
