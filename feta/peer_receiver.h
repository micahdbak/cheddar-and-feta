#pragma once

#include "game.h"
#include "object.h"
#include "sprite.h"

class PeerReceiver : public thoom::Object {
 public:
  PeerReceiver() = default;
  ~PeerReceiver();

  void step() override;

 private:
  bool handle_important_message(const char* arr);
  void load_map(const std::string& map, const char* remaining_msgs);

  std::vector<thoom::SpriteRender> sprites;
  std::string skipped_messages;
  Uint64 last_frame_ticks = 0;
  bool just_pushed_feta = false, just_loaded_map = false;
};

class PeerReceiverFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    return new PeerReceiver();
  }
};
