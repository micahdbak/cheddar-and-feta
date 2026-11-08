#pragma once

#include <string>
#include <vector>

#include "hitbox.h"
#include "mouse.h"
#include "object.h"
#include "scene/scene.h"

#define CHEDDAR_OBJ "cheddar"

class Cheddar : public Mouse, public HitSource {
 public:
  explicit Cheddar(std::vector<Mouse::SpawnCoord>& coordinates);
  ~Cheddar();

  void render() override;

  // Mouse
  void signal_down() override;

  // HitSource
  float hitsource_x() override { return this->x; }
  float hitsource_y() override { return this->y; }
  void hitsource_notify() override { this->did_hit = true; }
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 protected:
  void on_down() override;
};
