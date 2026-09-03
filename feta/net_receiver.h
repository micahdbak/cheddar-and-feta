#ifndef NET_RECEIVER_H
#define NET_RECEIVER_H

#include "game.h"
#include "object.h"

class NetReceiver : public Object {
 public:
  NetReceiver() = default;
  ~NetReceiver();

  void step() override;

 private:
  bool handle_important_message(const char* arr);
  void load_map(const std::string& map, const char* remaining_msgs);

  std::vector<Game::SpriteRender> sprites;
  std::string skipped_messages;
  Uint64 last_frame_ticks = 0;
  bool just_pushed_feta = false, just_loaded_map = false;
};

class NetReceiverFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) override {
    return new NetReceiver();
  }
};

#endif
