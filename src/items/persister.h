#pragma once

#include <queue>
#include <string>
#include <unordered_map>

#include "object.h"
#include "scene/scene.h"
#include "thoom/thoom.h"

#define ITEM_PERSISTER_OBJ "item_persister"

class ItemPersister : public thoom::Object {
 public:
  ItemPersister();
  ~ItemPersister();

  void step() override;

  void remember_item(const std::string& item_id, float x, float y,
                     thoom_gid gid);
  void forget_item(thoom_gid gid);

  bool creating_items = false;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  void save_items();

  struct ItemSave {
    std::string item_id;
    float x, y;
    thoom_gid gid;
  };

  std::unordered_map<thoom_gid, ItemSave> items_to_save;
  std::queue<ItemSave> items_to_spawn;
};

extern ItemPersister* item_persister;
