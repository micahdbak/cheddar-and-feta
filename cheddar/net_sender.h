#ifndef NET_SENDER_H
#define NET_SENDER_H

#include <string>
#include <vector>

#include "object.h"

class NetSender : public Object {
 public:
  NetSender();
  ~NetSender();

  void step() override;

  static void send_message(char func, const std::string& arg);

 private:
  std::vector<std::string> messages;
  bool was_disconnected = true;
};

class NetSenderFactory : public ObjectFactory {
 public:
  Object* create(const std::string& options) override {
    return new NetSender();
  }
};

#endif
