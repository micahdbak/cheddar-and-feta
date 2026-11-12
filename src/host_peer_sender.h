#pragma once

#include <string>
#include <vector>

#include "object/object.h"
#include "peer/peer.h"

#define PEER_SENDER_OBJ "peer_sender"

class HostPeerSender : public thoom::Object {
 public:
  HostPeerSender();
  ~HostPeerSender();

  void step() override;
  void render() override;

  static void send_message(char func, const std::string& arg);
  static thoom::Object* create_object(const std::string& options);

 private:
  std::vector<std::string> messages;
  bool was_disconnected = true;
  thoom::Peer* peer = thoom::Peer::instance;
};
