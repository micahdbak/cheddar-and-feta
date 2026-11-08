#include "guest_peer_sender.h"

#include <cstdio>
#include <string>

#include "guest_feta.h"
#include "object.h"
#include "peer/peer.h"
#include "save/save_data.h"
#include "scene/scene.h"

GuestPeerSender* guest_peer_sender;

GuestPeerSender::GuestPeerSender() { guest_peer_sender = this; }

GuestPeerSender::~GuestPeerSender() { guest_peer_sender = nullptr; }

void GuestPeerSender::step() {
  if (guest_feta == nullptr) {
    return;
  }

  char x_str[256], y_str[256], buff[1024];
  std::string frame_msg;

  thoom::float_to_str(guest_feta->x, x_str, sizeof(x_str));
  thoom::float_to_str(guest_feta->y, y_str, sizeof(y_str));

  snprintf(buff, sizeof(buff), "%c%s,%s,%d,%d,%d\n", MSG_FETA_INFO, x_str,
           y_str, guest_feta->sprite->animation, guest_feta->sprite->frame_i,
           guest_feta->which_emote);

  frame_msg += buff;

  for (int i = 0; i < this->messages.size(); i++) {
    frame_msg += this->messages[i];
  }

  this->messages.clear();

  thoom::Peer::instance->send_queue.push(frame_msg);
}

void GuestPeerSender::send_message(char func, const std::string& arg) {
  if (guest_peer_sender == nullptr) {
    return;
  }

  if (thoom::Peer::instance->get_state() != thoom::Peer::kStateConnected) {
    if (!guest_peer_sender->messages.empty()) {
      guest_peer_sender->messages.clear();
    }

    return;
  }

  guest_peer_sender->messages.push_back(std::string(func + arg + '\n'));
}

void GuestPeerSender::send_audio(const char* wav_path, float gain, float x,
                                 float y) {
  char buff[256];
  snprintf(buff, sizeof(buff), "%s %d,%d,%d\n", wav_path, (int)(10.0f * gain),
           (int)x, (int)y);
  GuestPeerSender::send_message(MSG_AUDIO, std::string(buff));
}

thoom::Object* GuestPeerSender::create_object(thoom::Scene* scene,
                                              const std::string& options) {
  return new GuestPeerSender();
}
