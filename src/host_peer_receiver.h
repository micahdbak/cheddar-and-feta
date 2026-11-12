#pragma once

#include <string>

#include "object/object.h"
#include "peer/peer.h"

#define PEER_RECEIVER_OBJ "peer_receiver"

class HostPeerReceiver : public thoom::Object {
 public:
  HostPeerReceiver() : thoom::Object(nullptr) {}
  ~HostPeerReceiver() = default;

  void step() override;
  static thoom::Object* create_object(const std::string& options);

 private:
  thoom::Peer* peer = thoom::Peer::instance;
};
