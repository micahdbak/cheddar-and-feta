#pragma once

#include <queue>
#include <string>
#include <vector>

#include "object.h"
#include "peer/peer.h"
#include "renderer/renderer.h"
#include "scene/scene.h"

#define GUEST_PEER_RECEIVER_OBJ "guest_peer_receiver"

class GuestPeerReceiver : public thoom::Object {
 public:
  GuestPeerReceiver() = default;
  ~GuestPeerReceiver();

  void step() override;
  void render() override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  bool handle_important_message(const char* arr);
  void load_map(const std::string& map, const char* remaining_msgs);
  void clear_sprites();

  std::vector<thoom::SpriteRender> sprites;
  std::queue<std::string> pending_messages;
  std::string skipped_messages;
  Uint64 last_frame_ticks = 0;
  bool just_pushed_feta = false, just_loaded_map = false;

  thoom::Peer* peer = thoom::Peer::instance;
};
