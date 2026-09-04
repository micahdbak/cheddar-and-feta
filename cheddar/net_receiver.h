#pragma once

#include "object.h"

class NetReceiver : public thoom::Object {
 public:
  NetReceiver() = default;
  ~NetReceiver() = default;

  void step() override;
};

class NetReceiverFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    return new NetReceiver();
  }
};
