#pragma once

#include <queue>
#include <unordered_map>

#include "object.h"
#include "scene.h"

#define ITEM_PERSISTER_OBJ "item_persister"

class ItemPersister : public thoom::Object {
 public:
  ItemPersister();
  ~ItemPersister();

  void step() override;

  void remember_item(const std::string& item_id, float x, float y, int id);
  void forget_item(int id);

  bool creating_items = false;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  void save_items();

  struct ItemSave {
    std::string item_id;
    float x, y;
    int id;
  };

  std::unordered_map<int, ItemSave> items_to_save;
  std::queue<ItemSave> items_to_spawn;
};

extern ItemPersister* item_persister;
