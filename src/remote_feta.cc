#include "remote_feta.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "host_peer_sender.h"
#include "hurtbox.h"
#include "items/item.h"
#include "mouse.h"
#include "object/entity.h"
#include "peer/peer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

#define FETA_SYNC_RETRY_MS 500
#define FETA_SYNC_TIMEOUT_MS 2000

RemoteFeta::RemoteFeta(const thoom::Entity::Context& context, int x, int y,
                       int animation)
    : Mouse(context, "sprites/feta.bmp", true, FETA_OBJ) {
  if (remote_feta != nullptr) {
    // TODO(micahdbak): error
    std::exit(1);
  }
  remote_feta = this;

  std::string items_s = thoom::save.value(this->save_id + MOUSE_ITEMS);
  if (!items_s.empty()) {
    this->items = Mouse::read_items(items_s, require_item);
  }

  // if loading a save, read the location from the save file
  if (thoom::save.geti(LOAD_SAVE) && thoom::save.has(this->save_id + MOUSE_X)) {
    this->sprite->set_animation(
        thoom::save.geti(this->save_id + MOUSE_ANIMATION));
    this->place(thoom::save.getf(this->save_id + MOUSE_X),
                thoom::save.getf(this->save_id + MOUSE_Y));
  } else {
    // cheddar will have provided the corresponding coordinates and animation
    this->sprite->set_animation(animation);
    this->place((float)x, (float)y);
  }

  new HurtBox(*this, -8, -8, 16, 12);
}

RemoteFeta::~RemoteFeta() { remote_feta = nullptr; }

void RemoteFeta::step() {
  if (thoom::Peer::instance->get_state() != thoom::Peer::kStateConnected) {
    this->synchronized = false;
    this->sync_sent_ticks = 0;
    this->sprite->set_animation(SLEEPING_ANIMATION);
    return;
  }

  Uint64 ticks = thoom::Scene::instance->ticks;

  // The guest confirms the sync with its feta info. Resend the sync until it
  // does, and again if the confirmation stops because the guest lost Feta.
  if (this->synchronized &&
      ticks - this->last_feta_info_ticks > FETA_SYNC_TIMEOUT_MS) {
    this->synchronized = false;
  }

  if (!this->synchronized &&
      (this->sync_sent_ticks == 0 ||
       ticks - this->sync_sent_ticks > FETA_SYNC_RETRY_MS)) {
    this->synchronize();
    this->sync_sent_ticks = ticks;
  }

  this->update_path();
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
  items_s = Mouse::encode_items(this->items);

  snprintf(buff, sizeof(buff), "%s %s,%s,%d %s",
           thoom::Scene::instance->map->path.c_str(), x_str, y_str,
           this->sprite->animation, items_s.c_str());

  HostPeerSender::send_message(MSG_SYNC, std::string(buff));
}

thoom::Entity* RemoteFeta::create_entity(const thoom::Entity::Context& context,
                                         const std::string& options) {
  int x, y, animation;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &animation)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new RemoteFeta(context, x, y, animation);
}
