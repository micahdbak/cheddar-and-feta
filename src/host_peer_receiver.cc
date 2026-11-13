#include "host_peer_receiver.h"

#include <cstdio>
#include <queue>
#include <string>

#include "audio/audio.h"
#include "hitbox.h"
#include "items/cheese.h"
#include "items/coffee.h"
#include "items/item.h"
#include "items/save.h"
#include "items/tossed.h"
#include "mouse.h"
#include "object/entity.h"
#include "object/object.h"
#include "peer/peer.h"
#include "remote_feta.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/log.h"
#include "utils/utils.h"

void HostPeerReceiver::step() {
  if (this->peer == nullptr || thoom::Peer::instance != this->peer) {
    this->deleted = true;
    return;
  }
  std::queue<std::string> msgs = thoom::Peer::instance->recv_queue.take_all();
  char buff[1024];

  // shouldn't be receiving any messages; ignore
  if (remote_feta == nullptr) {
    return;
  }

  RemoteFeta* remote_feta_inst = dynamic_cast<RemoteFeta*>(remote_feta);

  if (remote_feta_inst == nullptr) {
    return;
  }

  while (!msgs.empty()) {
    std::string msg = msgs.front();
    msgs.pop();

    for (const char* arr = msg.c_str(); *arr != '\0';
         arr = thoom::next_line(arr)) {
      // The feta info is the guest's acknowledgement of the sync, so it is
      // the only message read before the guest has a Feta.
      if (!remote_feta_inst->synchronized && arr[0] != MSG_FETA_INFO) {
        continue;
      }

      switch (arr[0]) {
        case MSG_USE_ITEM: {
          int x, y, x_dir, y_dir;
          if (5 != sscanf(arr + 1, "%1023s %d,%d,%d,%d", buff, &x, &y, &x_dir,
                          &y_dir)) {
            thoom_log_warn("HostPeerReceiver: bad use-item message");
            break;
          }

          std::string options =
              UseItem::options(x, y, x_dir, y_dir, remote_feta->gid);
          if (std::string(buff) == ITEM_SAVE) {
            thoom::Scene::instance->create_object(std::string(buff) + USE_OBJ,
                                                  options);
          } else if (std::string(buff) == ITEM_COFFEE_BEAN) {
            new UsedCoffeeBean(*remote_feta);
          } else {
            thoom::Scene::instance->create_entity(
                std::string(buff) + USE_OBJ, options,
                {thoom::Entity::Ownership::kFixed, remote_feta->owner_pid});
          }
          remote_feta->remove_item(std::string(buff));
        } break;

        case MSG_TOSS_ITEM: {
          int x, y, x_dir, y_dir;
          if (5 != sscanf(arr + 1, "%1023s %d,%d,%d,%d", buff, &x, &y, &x_dir,
                          &y_dir)) {
            thoom_log_warn("HostPeerReceiver: bad toss-item message");
            break;
          }

          std::string options =
              TossedItem::options(x, y, x_dir, y_dir, true, buff);
          thoom::Scene::instance->create_entity(
              TOSSED_ITEM_OBJ, options,
              {thoom::Entity::Ownership::kFixed, remote_feta->owner_pid});
          remote_feta->remove_item(std::string(buff));
        } break;

        case MSG_PUSH_HITBOX: {
          int x_dir, y_dir, x_off, y_off;
          if (3 != sscanf(arr + 1, "%1023s %d,%d", buff, &x_dir, &y_dir)) {
            thoom_log_warn("HostPeerReceiver: bad hitbox message");
            break;
          }

          std::string sel_item_id = ITEM_NONE;
          if (item_info.find(std::string(buff)) != item_info.end()) {
            sel_item_id = buff;
          }

          HitBox::make_offset(x_dir, y_dir, &x_off, &y_off, 8.0f);
          HitBox::Properties props = {item_info[sel_item_id].damage, 200, 100};
          props.single_use = true;

          new HitBox(*remote_feta, remote_feta, x_off - 16, y_off - 18, 32, 32,
                     props);
        } break;

        case MSG_FETA_INFO: {
          char map[1024];
          char x_str[256], y_str[256];
          int animation, frame_i, which_emote;
          if (6 != sscanf(arr + 1, "%1023s %255[^,],%255[^,],%d,%d,%d", map,
                          x_str, y_str, &animation, &frame_i, &which_emote)) {
            thoom_log_warn("HostPeerReceiver: bad feta-info message");
            break;
          }
          map[1023] = x_str[255] = y_str[255] = '\0';

          // Feta info from before a map change describes the previous map.
          if (thoom::Scene::instance->map->path != map) {
            break;
          }

          remote_feta->x = thoom::str_to_float(x_str);
          remote_feta->y = thoom::str_to_float(y_str);
          remote_feta_inst->sprite->set_animation(animation);
          remote_feta_inst->sprite->set_frame(frame_i);
          remote_feta_inst->which_emote = which_emote;
          remote_feta_inst->synchronized = true;
          remote_feta_inst->last_feta_info_ticks =
              thoom::Scene::instance->ticks;
        } break;

        case MSG_EAT_CHEESE:
          remote_feta->remove_item(ITEM_CHEESE);
          break;

        case MSG_AUDIO: {
          int gain_i, x, y;
          float gain;

          if (4 != sscanf(arr + 1, "%1023s %d,%d,%d", buff, &gain_i, &x, &y)) {
            thoom_log_warn("HostPeerReceiver: bad audio message");
            break;
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
          thoom_log_warn("HostPeerReceiver: unknown message '%c'", arr[0]);
          break;
      }
    }
  }
}

thoom::Object* HostPeerReceiver::create_object(const std::string& options) {
  return new HostPeerReceiver();
}
