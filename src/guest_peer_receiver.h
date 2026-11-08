#pragma once

#include <string>
#include <vector>

#include "object.h"
#include "renderer/renderer.h"
#include "scene/scene.h"

#define PEER_RECEIVER_OBJ "peer_receiver"

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

  std::vector<thoom::SpriteRender> sprites;
  std::string skipped_messages;
  Uint64 last_frame_ticks = 0;
  bool just_pushed_feta = false, just_loaded_map = false;
};
