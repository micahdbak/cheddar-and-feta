#pragma once

#include <string>
#include <vector>

#include "hitbox.h"
#include "mouse.h"
#include "object/entity.h"

#define CHEDDAR_OBJ "cheddar"

class Cheddar : public Mouse, public HitSource {
 public:
  explicit Cheddar(const thoom::Entity::Context& context,
                   std::vector<Mouse::SpawnCoord>& coordinates);
  ~Cheddar();

  void render() override;

  // Mouse
  void signal_down() override;

  // HitSource
  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }
  void hitsource_notify() override { this->did_hit = true; }
  static thoom::Entity* create_entity(const thoom::Entity::Context& context,
                                      const std::string& options);

 protected:
  void on_down() override;
};
