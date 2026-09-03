#ifndef HITBOX_OBJ
#define HITBOX_OBJ "hitbox"

#include "game.h"
#include "object.h"

// hitbox shared IDs
#define HB_SHARED_UNKNOWN -1
#define HB_SHARED_FIRE -2
#define HB_SHARED_SPITTER -3

class HitSource {
 public:
  HitSource() = default;
  ~HitSource() = default;

  virtual float hitsource_x() = 0;
  virtual float hitsource_y() = 0;
  virtual void hitsource_notify() {};
};

class HitBox : public Object {
 public:
  struct Properties {
    int damage;
    int cooldown_ms;
    int delete_after_ms = 0;
    bool shared_cooldowns = false;
    int shared_id = HB_SHARED_UNKNOWN;
    bool single_use = false;
  };

  static void MakeOffset(int x_dir, int y_dir, int* x_off, int* y_off,
                         float distance) {
    bool is_diagonal = x_dir != 0 && y_dir != 0;
    int hitbox_distance =
        (int)(is_diagonal ? DIAG_MULTIPLIER * distance : distance);
    *x_off = x_dir * hitbox_distance;
    *y_off = y_dir * hitbox_distance;
  }

  static std::string Options(int hitsource_owner_id, int hurtbox_owner_id,
                             int x_off, int y_off, int w, int h,
                             Properties props) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
             hitsource_owner_id, hurtbox_owner_id, x_off, y_off, w, h,
             props.damage, props.cooldown_ms, props.delete_after_ms,
             props.shared_cooldowns ? 1 : 0, props.shared_id,
             props.single_use ? 1 : 0);
    return std::string(buff);
  }

  HitBox(int hurtbox_owner_id, HitSource* source,
         std::shared_ptr<bool> source_deleted, int x_off, int y_off, int w,
         int h, Properties props);
  ~HitBox();

  void step() override;

  SDL_Rect bounding_box;

  bool throw_away = true;

  bool shared_cooldowns = false, single_use = false;
  int shared_id = HB_SHARED_UNKNOWN;

 private:
  bool from_foe;
  HitSource* source;
  std::shared_ptr<bool> source_deleted;
  int hurtbox_owner_id, x_off, y_off, damage, cooldown_ms, delete_after_ms;
  uint64_t created_ticks;

  SDL_FRect src_rect, dst_rect;
};

class HitBoxFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) {
    int hitsource_owner_id, hurtbox_owner_id, x_off, y_off, w, h, damage,
        cooldown_ms, delete_after_ms, shared_cooldowns, shared_id, single_use;
    if (12 != sscanf(options.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                     &hitsource_owner_id, &hurtbox_owner_id, &x_off, &y_off, &w,
                     &h, &damage, &cooldown_ms, &delete_after_ms,
                     &shared_cooldowns, &shared_id, &single_use))
      FATAL_ERROR

    Object* source_obj = game->get_object(hitsource_owner_id);
    if (source_obj == nullptr) return nullptr;

    std::shared_ptr<bool> source_deleted = source_obj->get_deleted_ptr();

    HitSource* source = dynamic_cast<HitSource*>(source_obj);
    if (source == nullptr) return nullptr;

    HitBox::Properties props;
    props.damage = damage;
    props.cooldown_ms = cooldown_ms;
    props.delete_after_ms = delete_after_ms;
    props.shared_cooldowns = shared_cooldowns == 1;
    props.shared_id = shared_id;
    props.single_use = single_use == 1;

    return new HitBox(hurtbox_owner_id, source, source_deleted, x_off, y_off, w,
                      h, props);
  }
};

#endif