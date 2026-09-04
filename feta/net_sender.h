#pragma once

#include "game.h"
#include "object.h"

class NetSender : public thoom::Object {
 public:
  NetSender();
  ~NetSender();

  void step() override;

  static void send_message(char func, const std::string& arg);

 private:
  std::vector<std::string> messages;
};

class NetSenderFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    return new NetSender();
  }
};
