#include "net_sender.h"

#include "game.h"
#include "items/item.h"
#include "mouse.h"
#include "net_agent.h"

NetSender* net_sender = nullptr;

NetSender::NetSender() { net_sender = this; }

NetSender::~NetSender() { net_sender = nullptr; }

void NetSender::step() {
  if (net_agent->get_state() != NetworkAgent::State::CONNECTED) return;

  std::string frame_msg = MSG_MAP + game->current_map + '\n';
  char buff[1024];

  // sprites
  for (Game::SpriteRender& sprite : game->sprites) {
    if (sprite.texture == nullptr || sprite.dst_rect == nullptr ||
        sprite.tex_id.size() == 0)
      continue;

    SDL_Rect src_rect, dst_rect;

    if (sprite.src_rect != nullptr) {
      src_rect = SDL_Rect{(int)sprite.src_rect->x, (int)sprite.src_rect->y,
                          (int)sprite.src_rect->w, (int)sprite.src_rect->h};
    } else {
      src_rect = SDL_Rect{0, 0, (int)sprite.texture->w, (int)sprite.texture->h};
    }

    dst_rect = SDL_Rect{(int)sprite.dst_rect->x, (int)sprite.dst_rect->y,
                        (int)sprite.dst_rect->w, (int)sprite.dst_rect->h};

    snprintf(buff, sizeof(buff), "%c%s %d,%d,%d,%d %d,%d,%d,%d %d\n",
             MSG_SPRITE, sprite.tex_id.c_str(), src_rect.x, src_rect.y,
             src_rect.w, src_rect.h, dst_rect.x, dst_rect.y, dst_rect.w,
             dst_rect.h, sprite.y - dst_rect.y);
    frame_msg += buff;
  }

  // audio
  for (Game::AudioMsg& msg : game->audio) {
    snprintf(buff, sizeof(buff), "%c%s %d,%d,%d\n", MSG_AUDIO,
             msg.wav_path.c_str(), (int)(10.0f * msg.gain), (int)msg.x,
             (int)msg.y);
    frame_msg += buff;
  }

  for (const std::string& msg : this->messages) {
    frame_msg += msg;
  }

  this->messages.clear();

  net_agent->send_message(frame_msg);
}

void NetSender::send_message(char func, const std::string& arg) {
  if (net_sender == nullptr) {
    return;
  }

  if (game->net_state != NetworkAgent::State::CONNECTED) {
    if (!net_sender->messages.empty()) {
      // don't want a clogged up message queue
      net_sender->messages.clear();
    }

    return;
  }

  net_sender->messages.push_back(std::string(func + arg + '\n'));
}
