#ifndef FOE_SPITTER_ABDOMEN_OBJ
#define FOE_SPITTER_ABDOMEN_OBJ "foe_spitter_abdomen"

#include "hurtbox.h"
#include "spitter.h"
#include "sprite.h"

#define FRAMES_TO_SET_CUR_DIR 4

class SpitterAbdomen : public Object, public HurtSource {
 public:
  SpitterAbdomen(Spitter* parent, std::shared_ptr<bool> deleted_ptr);
  ~SpitterAbdomen();

  void step() override;

  float hurtsource_x() override { return this->x; }
  float hurtsource_y() override { return this->y; }

  void attack(int damage) override {
    if (!*this->deleted_ptr) {
      this->parent->attack(damage);
    }
  }

 private:
  int cur_dir, dirs[FRAMES_TO_SET_CUR_DIR], dirs_i;
  float x, y;
  Spitter* parent;
  std::shared_ptr<bool> deleted_ptr;
  Sprite* sprite;
  SDL_FRect dst_rect;
};

class SpitterAbdomenFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) {
    int parent_id;
    if (1 != sscanf(options.c_str(), "%d", &parent_id)) FATAL_ERROR

    Object* obj = game->get_object(parent_id);
    Spitter* spitter;

    if (obj != nullptr && (spitter = dynamic_cast<Spitter*>(obj)) != nullptr) {
      std::shared_ptr<bool> deleted_ptr = spitter->get_deleted_ptr();
      return new SpitterAbdomen(spitter, deleted_ptr);
    } else {
      return nullptr;
    }
  }
};

#endif