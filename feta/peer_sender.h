#pragma once

#include "game.h"
#include "object.h"

class PeerSender : public thoom::Object {
 public:
  PeerSender();
  ~PeerSender();

  void step() override;

  static void send_message(char func, const std::string& arg);

 private:
  std::vector<std::string> messages;
};

class PeerSenderFactory : public thoom::ObjectFactory {
 public:
  thoom::Object* create(const std::string& options) override {
    return new PeerSender();
  }
};
