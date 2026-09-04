#pragma once

#include "game.h"
#include "mouse.h"
#include "object.h"
#include "save_data.h"

#define ITEM_SAVE "item_save"

class ItemSaveUse : public thoom::Object {
 public:
  ItemSaveUse(bool is_feta) : is_feta(is_feta) {}
  ~ItemSaveUse() = default;

  void step() {
    // feta shouldnt trigger saves - theyre a remote player, seems insecure
    if (this->is_feta) {
      thoom::game->delete_object = true;
      return;
    }

    if (cheddar != nullptr) cheddar->push_item(ITEM_SAVE);

    thoom::game->save_objects();
    thoom::game->display_notification(
        "Saved to file " + std::to_string(thoom::save.geti(SAVE_FILE) + 1) +
        ".");
    thoom::save.write_file(thoom::save.geti(SAVE_FILE));

    // all done
    thoom::game->delete_object = true;
  }

 private:
  bool is_feta;
};

class ItemSaveUseFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) {
    int obj_id;
    if (1 != sscanf(options.c_str(), "%*d,%*d,%*d,%*d,%d", &obj_id)) FATAL_ERROR

    bool is_feta = feta != nullptr && feta->id == obj_id;

    return new ItemSaveUse(is_feta);
  }
};
