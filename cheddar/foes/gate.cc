#include "gate.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "audio.h"
#include "object.h"
#include "peer.h"
#include "peer_sender.h"
#include "renderer.h"
#include "save_data.h"
#include "scene.h"
#include "spawner.h"
#include "sprite.h"

FoeGate::FoeGate(int x, int y, int spawner_id, int animation)
    : spawner_id(spawner_id) {
  this->sprite = new thoom::Sprite("sprites/gate.bmp", 48, 48, 0);
  this->dst_rect.w = 48.0f;
  this->dst_rect.h = 48.0f;
  this->sprite->set_animation(animation);

  x = x - (x % thoom::Scene::instance->map->tile_width);
  y = y - (y % thoom::Scene::instance->map->tile_height);
  this->x = float(x);
  this->y = float(y);

  this->colliders_offset =
      ((y / thoom::Scene::instance->map->tile_height) + 1) *
      thoom::Scene::instance->map->cols;
  this->colliders_x = x / thoom::Scene::instance->map->tile_width;

  char key[256];
  snprintf(key, sizeof(key), "spawner_%d", this->spawner_id);
  if (thoom::save.geti(key) == 1) {
    this->sprite->set_frame(1);
    this->opened = true;
    return;
  }

  /* gates are 48x48; tiles are 16x16; therefore a gate should look like:
   * @ - + - + - + <- @ is gate coordinate
   * | O | O | O |
   * + - + - + - +
   * | O | O | O |
   * + - + - + - +
   * | # | # | # | <- # is a collider
   */

  for (int i = 0; i < 3; i++) {
    int collider_x = this->colliders_x + i;
    int collider_y = this->colliders_offset / thoom::Scene::instance->map->cols;
    this->colliders.push_back(
        thoom::Scene::instance->map->collider->get(collider_x, collider_y));
    thoom::Scene::instance->map->collider->set(collider_x, collider_y,
                                               0);  // solid collider
  }
}

FoeGate::~FoeGate() { delete this->sprite; }

void FoeGate::step() {
  if (thoom::Peer::instance->state.load() != thoom::Peer::kStateConnected) {
    this->synchronized = false;
  } else if (!this->synchronized) {
    for (int i = 0; i < 3; i++) {
      int collider_x = this->colliders_x + i;
      int collider_y =
          this->colliders_offset / thoom::Scene::instance->map->cols;
      int coord = this->colliders_offset + this->colliders_x + i;
      PeerSender::send_message(
          MSG_COLLISION,
          std::to_string(coord) + ',' +
              std::to_string(thoom::Scene::instance->map->collider->get(
                  collider_x, collider_y)));
    }

    this->synchronized = true;
  }

  if (!this->opened && spawners[this->spawner_id] != nullptr &&
      spawners[this->spawner_id]->empty) {
    // remove colliders
    for (int i = 0; i < 3; i++) {
      int collider_x = this->colliders_x + i;
      int collider_y =
          this->colliders_offset / thoom::Scene::instance->map->cols;
      thoom::Scene::instance->map->collider->set(collider_x, collider_y,
                                                 this->colliders[i]);
    }

    this->colliders.clear();

    this->sprite->set_frame(1);  // opened frame
    this->opened = true;

    this->synchronized = false;

    thoom::Audio::instance->play("sfx/door.wav", 1.0f, this->x, this->y, false);
  }

  this->dst_rect.x = this->x;
  this->dst_rect.y = this->y;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 24);
}

thoom::Object* FoeGate::create_object(thoom::Scene* scene,
                                      const std::string& options) {
  int x = 0, y = 0, spawner_id = 0, animation = 0;
  if (4 !=
      sscanf(options.c_str(), "%d,%d,%d,%d", &x, &y, &spawner_id, &animation)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new FoeGate(x, y, spawner_id, animation);
}
