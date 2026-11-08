#include "host_peer_receiver.h"

#include <cstdio>
#include <cstdlib>
#include <queue>
#include <string>

#include "audio/audio.h"
#include "hitbox.h"
#include "items/cheese.h"
#include "items/item.h"
#include "items/tossed.h"
#include "mouse.h"
#include "object.h"
#include "peer/peer.h"
#include "remote_feta.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "utils/utils.h"

void HostPeerReceiver::step() {
  std::queue<std::string> msgs = thoom::Peer::instance->all_messages();
  char buff[1024];

  // shouldn't be receiving any messages; ignore
  if (remote_feta == nullptr) {
    return;
  }

  RemoteFeta* remote_feta_inst = dynamic_cast<RemoteFeta*>(remote_feta);

  // should not consider messages when not synchronized
  if (remote_feta_inst == nullptr || !remote_feta_inst->synchronized) {
    return;
  }

  while (!msgs.empty()) {
    std::string msg = msgs.front();
    msgs.pop();

    for (const char* arr = msg.c_str(); *arr != '\0';
         arr = thoom::next_line(arr)) {
      switch (arr[0]) {
        case MSG_USE_ITEM: {
          int x, y, x_dir, y_dir;
          if (5 != sscanf(arr + 1, "%1023s %d,%d,%d,%d", buff, &x, &y, &x_dir,
                          &y_dir)) {
            // TODO(micahdbak): error
            std::exit(1);
          }

          std::string options =
              UseItem::options(x, y, x_dir, y_dir, remote_feta->id);
          thoom::Scene::instance->create_object(std::string(buff) + USE_OBJ,
                                                options);
          remote_feta->remove_item(std::string(buff));
        } break;

        case MSG_TOSS_ITEM: {
          int x, y, x_dir, y_dir;
          if (5 != sscanf(arr + 1, "%1023s %d,%d,%d,%d", buff, &x, &y, &x_dir,
                          &y_dir)) {
            // TODO(micahdbak): error
            std::exit(1);
          }

          std::string options =
              TossedItem::options(x, y, x_dir, y_dir, true, buff);
          thoom::Scene::instance->create_object(TOSSED_ITEM_OBJ, options);
          remote_feta->remove_item(std::string(buff));
        } break;

        case MSG_PUSH_HITBOX: {
          int x_dir, y_dir, x_off, y_off;
          if (3 != sscanf(arr + 1, "%1023s %d,%d", buff, &x_dir, &y_dir)) {
            // TODO(micahdbak): error
            std::exit(1);
          }

          std::string sel_item_id = ITEM_NONE;
          if (item_info.find(std::string(buff)) != item_info.end()) {
            sel_item_id = buff;
          }

          HitBox::make_offset(x_dir, y_dir, &x_off, &y_off, 8.0f);
          HitBox::Properties props = {item_info[sel_item_id].damage, 200, 100};
          props.single_use = true;

          thoom::Scene::instance->create_object(
              HITBOX_OBJ,
              HitBox::options(remote_feta->id, remote_feta->id, x_off - 16,
                              y_off - 18, 32, 32, props));
        } break;

        case MSG_FETA_INFO: {
          char x_str[256], y_str[256];
          int animation, frame_i, which_emote;
          if (5 != sscanf(arr + 1, "%[^,],%[^,],%d,%d,%d", x_str, y_str,
                          &animation, &frame_i, &which_emote)) {
            // TODO(micahdbak): error
            std::exit(1);
          }

          remote_feta->x = thoom::str_to_float(x_str);
          remote_feta->y = thoom::str_to_float(y_str);
          remote_feta_inst->animation = animation;
          remote_feta_inst->frame_i = frame_i;
          remote_feta_inst->which_emote = which_emote;
        } break;

        case MSG_EAT_CHEESE:
          remote_feta->remove_item(ITEM_CHEESE);
          break;

        case MSG_AUDIO: {
          int gain_i, x, y;
          float gain;

          if (4 != sscanf(arr + 1, "%1023s %d,%d,%d", buff, &gain_i, &x, &y)) {
            // TODO(micahdbak): error
            std::exit(1);
          }

          gain = (float)gain_i / 10.0f;

          thoom::Audio::instance->play(std::string(buff), gain, (float)x,
                                       (float)y, true);
        } break;

        case MSG_IS_DOWN:
          remote_feta->is_down = arr[1] == '1' ? true : false;
          cheddar->signal_down();
          break;

        default:
          break;
      }
    }
  }
}

thoom::Object* HostPeerReceiver::create_object(thoom::Scene* scene,
                                               const std::string& options) {
  return new HostPeerReceiver();
}
