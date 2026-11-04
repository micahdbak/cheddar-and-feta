#pragma once

#include "object.h"

class PeerReceiver : public thoom::Object {
 public:
  PeerReceiver() = default;
  ~PeerReceiver() = default;

  void step() override;
};

class PeerReceiverFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    return new PeerReceiver();
  }
};
