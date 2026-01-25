#include "controller.h"
#include "feta.h"
#include "game.h"
#include "net_agent.h"
#include "net_sender.h"
#include "save_data.h"

NetSender *net_sender;

NetSender::NetSender() {
    net_sender = this;
}

NetSender::~NetSender() {
    net_sender = nullptr;
}

void NetSender::step() {
    if (feta == nullptr) {
        return;
    }

    char x_str[256], y_str[256], buff[1024];
    std::string frame_msg;

    float_to_str(feta->x, x_str, sizeof(x_str));
    float_to_str(feta->y, y_str, sizeof(y_str));

    snprintf(buff, sizeof(buff), "%c%s,%s,%d,%d,%d\n", MSG_FETA_INFO,
        x_str, y_str, feta->sprite->animation, feta->sprite->frame_i, feta->which_emote);

    frame_msg += buff;

    for (int i = 0; i < this->messages.size(); i++) {
        frame_msg += this->messages[i];
    }

    this->messages.clear();

    net_agent->send_message(frame_msg);
}

void NetSender::send_message(char func, const std::string &arg) {
    if (net_sender == nullptr) {
        return;
    }

    if (game->net_state != NetworkAgent::State::CONNECTED) {
        if (!net_sender->messages.empty()) {
            net_sender->messages.clear();
        }

        return;
    }

    net_sender->messages.push_back(std::string(func + arg + '\n'));
}