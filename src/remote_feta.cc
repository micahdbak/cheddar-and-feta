#include "remote_feta.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "host_peer_sender.h"
#include "hurtbox.h"
#include "mouse.h"
#include "object.h"
#include "peer/peer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

RemoteFeta::RemoteFeta(int x, int y, int animation)
    : Mouse("sprites/feta.bmp", true) {
  if (remote_feta != nullptr) {
    // TODO(micahdbak): error
    std::exit(1);
  }
  remote_feta = this;

  std::string items_s = thoom::save.value(FETA_OBJ MOUSE_ITEMS);
  if (!items_s.empty()) {
    this->items = IMouse::read_items(items_s);
  }

  // if loading a save, read the location from the save file
  if (thoom::save.geti(LOAD_SAVE) && thoom::save.has(FETA_OBJ MOUSE_X)) {
    this->sprite->set_animation(thoom::save.geti(FETA_OBJ MOUSE_ANIMATION));
    this->place(thoom::save.getf(FETA_OBJ MOUSE_X),
                thoom::save.getf(FETA_OBJ MOUSE_Y));
  } else {
    // cheddar will have provided the corresponding coordinates and animation
    this->sprite->set_animation(animation);
    this->place((float)x, (float)y);
  }

  // push hurtbox
  thoom::Scene::instance->create_object(
      HURTBOX_OBJ, HurtBox::options(this->id, -8, -8, 16, 12));
}

RemoteFeta::~RemoteFeta() { remote_feta = nullptr; }

void RemoteFeta::step() {
  if (thoom::Peer::instance->state.load() != thoom::Peer::kStateConnected) {
    this->synchronized = false;
    this->sprite->set_animation(SLEEPING_ANIMATION);
    this->draw();
    return;
  }

  if (!this->synchronized) {
    this->synchronize();
    this->synchronized = true;
  }

  this->update_path();
  this->draw();
}

void RemoteFeta::save_data() {
  thoom::save.putf(FETA_OBJ MOUSE_X, this->x);
  thoom::save.putf(FETA_OBJ MOUSE_Y, this->y);
  thoom::save.puti(FETA_OBJ MOUSE_ANIMATION, this->sprite->animation);
  thoom::save.data[FETA_OBJ MOUSE_ITEMS] = IMouse::encode_items(this->items);
}

void RemoteFeta::attack(int damage) {
  HostPeerSender::send_message(MSG_ATTACK, std::to_string(damage));
}

void RemoteFeta::push_item(const std::string& item_id) {
  HostPeerSender::send_message(MSG_PUSH_ITEM, item_id);
  Mouse::push_item(item_id);
}

void RemoteFeta::push_cheese(int amount) {
  HostPeerSender::send_message(MSG_PUSH_CHEESE, std::to_string(amount));
  Mouse::push_cheese(amount);
}

void RemoteFeta::remove_item(const std::string& item_id) {
  Mouse::remove_item(item_id);
}

void RemoteFeta::force_dance(uint64_t timeout_ms) {
  HostPeerSender::send_message(MSG_FORCE_DANCE, std::to_string(timeout_ms));
}

void RemoteFeta::set_throw(int throw_x, int throw_y) {
  HostPeerSender::send_message(
      MSG_THROW, std::to_string(throw_x) + ',' + std::to_string(throw_y));
}

void RemoteFeta::set_max_mov_speed(float max_mov_speed) {
  HostPeerSender::send_message(MSG_MAX_SPEED, std::to_string(max_mov_speed));
}

void RemoteFeta::hitsource_notify() {
  HostPeerSender::send_message(MSG_DID_HIT, std::string());
}

void RemoteFeta::synchronize() {
  char buff[1024];
  char x_str[256], y_str[256];
  std::string items_s;

  thoom::float_to_str(this->x, x_str, sizeof(x_str));
  thoom::float_to_str(this->y, y_str, sizeof(y_str));
  items_s = IMouse::encode_items(this->items);

  snprintf(buff, sizeof(buff), "%s,%s,%d %s", x_str, y_str,
           this->sprite->animation, items_s.c_str());

  HostPeerSender::send_message(MSG_SYNC, std::string(buff));
}

thoom::Object* RemoteFeta::create_object(thoom::Scene* scene,
                                         const std::string& options) {
  int x, y, animation;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &animation)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new RemoteFeta(x, y, animation);
}
