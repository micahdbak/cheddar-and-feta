#include "controller.h"
#include "feta.h"
#include "game.h"
#include "items/cheese.h"
#include "items/item.h"
#include "items/tossed.h"
#include "net_agent.h"
#include "net_receiver.h"
#include "audio_playback.h"

void NetReceiver::step() {
    std::queue<std::string> msgs = net_agent->all_messages();
    char buff[1024];

    // shouldn't be receiving any messages; ignore
    if (feta == nullptr) {
        return;
    }

    Feta *feta_inst = dynamic_cast<Feta *>(feta);

    // should not consider messages when not synchronized
    if (feta_inst == nullptr || !feta_inst->synchronized) {
        return;
    }

    while (!msgs.empty()) {
        std::string msg = msgs.front();
        msgs.pop();
        
        for (const char *arr = msg.c_str(); *arr != '\0'; arr = next_line(arr)) {
            switch (arr[0]) {
            case MSG_USE_ITEM: {
                int x, y, x_dir, y_dir;
                if (5 != sscanf(arr + 1, "%1023s %d,%d,%d,%d", buff, &x, &y, &x_dir, &y_dir)) FATAL_ERROR

                std::string options = UseItem::Options(x, y, x_dir, y_dir, feta->id);
                game->push_object(std::string(buff) + USE_OBJ, options);
                feta->remove_item(std::string(buff));
            } break;

            case MSG_TOSS_ITEM: {
                int x, y, x_dir, y_dir;
                if (5 != sscanf(arr + 1, "%1023s %d,%d,%d,%d", buff, &x, &y, &x_dir, &y_dir)) FATAL_ERROR
                
                std::string options = TossedItem::Options(x, y, x_dir, y_dir, true, buff);
                game->push_object(TOSSED_ITEM_OBJ, options);
                feta->remove_item(std::string(buff));
            } break;

            case MSG_PUSH_HITBOX: {
                int x_dir, y_dir, x_off, y_off;
                if (3 != sscanf(arr + 1, "%1023s %d,%d", buff, &x_dir, &y_dir)) FATAL_ERROR

                std::string sel_item_id = ITEM_NONE;
                if (item_info.find(std::string(buff)) != item_info.end()) {
                    sel_item_id = buff;
                }

                HitBox::MakeOffset(x_dir, y_dir, &x_off, &y_off, 8.0f);
                HitBox::Properties props = {item_info[sel_item_id].damage, 200, 100};
                props.single_use = true;

                game->push_object(HITBOX_OBJ, HitBox::Options(feta->id, feta->id, x_off - 16, y_off - 18, 32, 32, props));
            } break;

            case MSG_FETA_INFO: {
                char x_str[256], y_str[256];
                int animation, frame_i;
                if (4 != sscanf(arr + 1, "%[^,],%[^,],%d,%d", x_str, y_str, &animation, &frame_i)) FATAL_ERROR

                feta->x = str_to_float(x_str);
                feta->y = str_to_float(y_str);
                feta_inst->animation = animation;
                feta_inst->frame_i = frame_i;
            } break;

            case MSG_EAT_CHEESE:
                feta->remove_item(ITEM_CHEESE);
                break;

            case MSG_AUDIO: {
                int gain_i, x, y;
                float gain;

                if (4 != sscanf(arr + 1, "%1023s %d,%d,%d", buff, &gain_i, &x, &y)) FATAL_ERROR

                gain = (float)gain_i / 10.0f;

                play_audio(std::string(buff), gain, (float)x, (float)y, true);
            } break;

            case MSG_IS_DOWN:
                feta->is_down = arr[1] == '1' ? true : false;
                cheddar->signal_down();
                break;

            default: break;
            }
        }
    }
}
