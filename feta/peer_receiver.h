#pragma once

#include <string>
#include <vector>

#include "object.h"
#include "scene.h"
#include "sprite.h"

#define PEER_RECEIVER_OBJ "peer_receiver"

class PeerReceiver : public thoom::Object {
 public:
  PeerReceiver() = default;
  ~PeerReceiver();

  void step() override;
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
