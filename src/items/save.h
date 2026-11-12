#pragma once

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "hud/hud.h"
#include "mouse.h"
#include "object/object.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "thoom/thoom.h"

#define ITEM_SAVE "item_save"

class ItemSaveUse : public thoom::Object {
 public:
  ItemSaveUse(bool is_feta) : thoom::Object(nullptr), is_feta(is_feta) {}
  ~ItemSaveUse() = default;

  void step() {
    // the remote feta should not trigger saves; it is a remote player
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
  static thoom::Object* create_object(const std::string& options) {
    thoom_gid obj_gid;
    if (1 != sscanf(options.c_str(), "%*d,%*d,%*d,%*d,%" SCNu64, &obj_gid)) {
      std::exit(1);
    }

    bool is_feta = remote_feta != nullptr && remote_feta->gid == obj_gid;

    return new ItemSaveUse(is_feta);
  }

 private:
  bool is_feta;
};
