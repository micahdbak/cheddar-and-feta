#include "abdomen.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "foes/spitter/spitter.h"
#include "hurtbox.h"
#include "mouse.h"
#include "object.h"
#include "renderer.h"
#include "scene.h"
#include "sprite.h"
#include "utils.h"

SpitterAbdomen::SpitterAbdomen(int parent_id) : parent_id(parent_id) {
  this->sprite =
      new thoom::Sprite("sprites/foe_spitter_abdomen.bmp", 32, 32, 250);
  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;
  thoom::Scene::instance->create_object(
      HURTBOX_OBJ, HurtBox::options(this->id, -16, -16, 32, 32));

  this->cur_dir = 0;
  for (int i = 0; i < FRAMES_TO_SET_CUR_DIR; i++) this->dirs[i] = 0;
  this->dirs_i = 0;
}

SpitterAbdomen::~SpitterAbdomen() { delete this->sprite; }

void SpitterAbdomen::attack(int damage) {
  thoom::Object* parent_object =
      thoom::Scene::instance->find_object(this->parent_id);
  Spitter* parent = dynamic_cast<Spitter*>(parent_object);

  if (parent != nullptr) parent->attack(damage);
}

void SpitterAbdomen::step() {
  thoom::Object* parent_object =
      thoom::Scene::instance->find_object(this->parent_id);
  Spitter* parent = dynamic_cast<Spitter*>(parent_object);

  if (parent == nullptr) {
    this->deleted = true;
    return;
  }

  struct Spitter::step_t p2, p3, p4;
  int direction;

  // p1, not used
  p2 = parent->get_pos(NUM_SEGMENTS);
  p3 = parent->get_pos(NUM_SEGMENTS - 1);
  p4 = parent->get_pos(NUM_SEGMENTS - 2);

  if (p2.x == 0 || p3.x == 0 || p4.x == 0) return;

  int this_x = p2.x;
  int this_y = p2.y;
  int next_x = (p2.x + p4.x) >> 1;
  int next_y = (p2.y + p4.y) >> 1;

  this->x = (float)this_x;
  this->y = (float)this_y;

  int x_dir, y_dir;
  thoom::dir_to_point(this_x, this_y, next_x, next_y, &x_dir, &y_dir);
  direction = thoom::direction_from_dirs(x_dir, y_dir);

  this->dirs[this->dirs_i] = direction;
  this->dirs_i = (++this->dirs_i) % FRAMES_TO_SET_CUR_DIR;
  int sum = 0;

  for (int i = 1; i < FRAMES_TO_SET_CUR_DIR; i++)
    sum += this->dirs[i - 1] == this->dirs[i] ? 1 : 0;

  if (sum >= FRAMES_TO_SET_CUR_DIR - 1) {
    this->cur_dir = this->dirs[0];
  }

  Mouse* mouse = Mouse::closest_mouse(this->x, this->y, 64.0f, true);

  if (mouse == nullptr) {
    parent->abdomen_distance = 256.0f;
  } else {
    parent->abdomen_distance =
        thoom::distance_between_points(mouse->x, mouse->y, this->x, this->y);
  }

  this->dst_rect.x = this->x - 16.0f;
  this->dst_rect.y = this->y - 16.0f;

  this->sprite->set_animation(this->cur_dir);
  this->sprite->update_frame();
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 16);
}

thoom::Object* SpitterAbdomen::create_object(thoom::Scene* scene,
                                             const std::string& options) {
  int parent_id;
  if (1 != sscanf(options.c_str(), "%d", &parent_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  thoom::Object* obj = scene->find_object(parent_id);
  if (obj == nullptr || dynamic_cast<Spitter*>(obj) == nullptr) return nullptr;

  return new SpitterAbdomen(parent_id);
}
