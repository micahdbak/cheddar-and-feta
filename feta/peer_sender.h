#pragma once

#include <string>
#include <vector>

#include "object.h"
#include "scene/scene.h"

#define PEER_SENDER_OBJ "peer_sender"

class PeerSender : public thoom::Object {
 public:
  PeerSender();
  ~PeerSender();

  void step() override;

  static void send_message(char func, const std::string& arg);
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  std::vector<std::string> messages;
};
