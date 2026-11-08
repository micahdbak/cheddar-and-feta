#pragma once

#include <string>

#include "object.h"
#include "scene/scene.h"

#define PEER_RECEIVER_OBJ "peer_receiver"

class HostPeerReceiver : public thoom::Object {
 public:
  HostPeerReceiver() = default;
  ~HostPeerReceiver() = default;

  void step() override;
  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);
};
