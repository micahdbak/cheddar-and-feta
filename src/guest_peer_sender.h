#pragma once

#include <string>
#include <vector>

#include "object/object.h"
#include "peer/peer.h"

#define GUEST_PEER_SENDER_OBJ "guest_peer_sender"

class GuestPeerSender : public thoom::Object {
 public:
  GuestPeerSender();
  ~GuestPeerSender();

  void step() override;

  static void send_message(char func, const std::string& arg);
  static void send_audio(const char* wav_path, float gain, float x, float y);
  static thoom::Object* create_object(const std::string& options);

 private:
  std::vector<std::string> messages;

  thoom::Peer* peer = thoom::Peer::instance;
};
