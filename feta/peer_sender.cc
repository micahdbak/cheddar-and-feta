#include "peer_sender.h"

#include <cstdio>
#include <string>

#include "feta.h"
#include "object.h"
#include "peer.h"
#include "save_data.h"
#include "scene.h"

PeerSender* peer_sender;

PeerSender::PeerSender() { peer_sender = this; }

PeerSender::~PeerSender() { peer_sender = nullptr; }

void PeerSender::step() {
  if (feta == nullptr) {
    return;
  }

  char x_str[256], y_str[256], buff[1024];
  std::string frame_msg;

  thoom::float_to_str(feta->x, x_str, sizeof(x_str));
  thoom::float_to_str(feta->y, y_str, sizeof(y_str));

  snprintf(buff, sizeof(buff), "%c%s,%s,%d,%d,%d\n", MSG_FETA_INFO, x_str,
           y_str, feta->sprite->animation, feta->sprite->frame_i,
           feta->which_emote);

  frame_msg += buff;

  for (int i = 0; i < this->messages.size(); i++) {
    frame_msg += this->messages[i];
  }

  this->messages.clear();

  thoom::Peer::instance->send_message(frame_msg);
}

void PeerSender::send_message(char func, const std::string& arg) {
  if (peer_sender == nullptr) {
    return;
  }

  if (thoom::Peer::instance->state.load() != thoom::Peer::kStateConnected) {
    if (!peer_sender->messages.empty()) {
      peer_sender->messages.clear();
    }

    return;
  }

  peer_sender->messages.push_back(std::string(func + arg + '\n'));
}

thoom::Object* PeerSender::create_object(thoom::Scene* scene,
                                         const std::string& options) {
  return new PeerSender();
}
