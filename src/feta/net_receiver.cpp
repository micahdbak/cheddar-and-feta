#include "bmp_texture.h"
#include "game.h"
#include "net_agent.h"
#include "net_receiver.h"
#include "audio_playback.h"
#include "feta.h"

#include <iostream>
#include <vector>
#include <string>

#define BUFF_SIZE 1024

char nullbyte = '\0';

NetReceiver::~NetReceiver() {
    if (!this->sprites.empty()) {
        for (Game::SpriteRender &sprite : this->sprites) {
            delete sprite.src_rect;
            delete sprite.dst_rect;
        }
        this->sprites.clear();
    }
}

void NetReceiver::step() {
    // ensure we are in maps/init if disconnected, and do nothing else
    if (game->net_state != NetworkAgent::State::CONNECTED) {
        // if currently in a map other than the init map, reset the network agent and go to that
        if (game->current_map != "maps/init") {
            net_agent->try_reset();
            game->map = "maps/init";
            last_frame_ticks = 0;
        }

        return;
    }

    this->just_pushed_feta = false;

    if (!this->skipped_messages.empty()) {
        std::string skipped_copy = this->skipped_messages;
        this->skipped_messages.clear();

        for (const char *arr = skipped_copy.c_str(); *arr != '\0'; arr = next_line(arr)) {
            this->handle_important_message(arr); // disregards other message types
        }
    }

    // get all unread messages
    std::queue<std::string> msgs = net_agent->all_messages();

    // if empty, reuse last frame
    if (msgs.empty()) {
        for (Game::SpriteRender &sprite : this->sprites) {
            game->push_sprite(sprite.tex_id, sprite.texture, sprite.src_rect, sprite.dst_rect, sprite.y);
        }

        // 5 seconds of no frames; disconnect
        if (game->ticks - last_frame_ticks > 5000 && last_frame_ticks != 0) {
            net_agent->try_reset();
            last_frame_ticks = 0;
        }

        return;
    }

    // set this as we have new frame(s) to parse
    last_frame_ticks = game->ticks;

    // free stuff
    for (Game::SpriteRender &sprite : this->sprites) {
        delete sprite.src_rect;
        delete sprite.dst_rect;
    }
    this->sprites.clear();

    char buff[1024];

    // read previous feta related messages
    while (msgs.size() > 1) {
        std::string msg = msgs.front();
        msgs.pop();

        const char *arr = msg.c_str();

        for (const char *arr = msg.c_str(); *arr != '\0'; arr = next_line(arr)) {
            if (arr[0] == MSG_MAP) {
                sscanf(arr + 1, "%1023[^\n]\n", buff);
                buff[1023] = '\0';

                if (game->current_map != buff) {
                    this->load_map(std::string(buff), next_line(arr));
                    return; // don't read rest of messages
                }
            }

            this->handle_important_message(arr); // disregards other message types
        }
    }

    // most recent message
    std::string msg = msgs.front();
    msgs.pop();

    for (const char *arr = msg.c_str(); *arr != '\0'; arr = next_line(arr)) {
        if (this->handle_important_message(arr)) {
            continue;
        }

        switch (arr[0]) {
        case MSG_MAP:
            sscanf(arr + 1, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            
            if (game->current_map != buff) {
                if (game->current_map != buff) {
                    this->load_map(std::string(buff), next_line(arr));
                    return; // don't read rest of messages
                }
            }

            break;

        case MSG_SPRITE: {
            if (this->just_loaded_map) {
                break;
            }

            SDL_Rect src_rect, dst_rect;
            int depth_offset;

            if (10 != sscanf(arr + 1, "%1023s %d,%d,%d,%d %d,%d,%d,%d %d",
                buff,
                &src_rect.x, &src_rect.y, &src_rect.w, &src_rect.h,
                &dst_rect.x, &dst_rect.y, &dst_rect.w, &dst_rect.h,
                &depth_offset)) FATAL_ERROR
            buff[1023] = '\0';

            Game::SpriteRender sprite;
            sprite.tex_id = buff; // buff is tex_id and sprite path

            // don't double render feta
            if (sprite.tex_id == "sprites/feta.bmp") {
                continue;
            }

            sprite.texture = load_bmp_texture(sprite.tex_id);

            if (sprite.texture == nullptr) {
                continue;
            }

            sprite.src_rect = new SDL_FRect(SDL_FRect{
                (float)src_rect.x,
                (float)src_rect.y,
                (float)src_rect.w,
                (float)src_rect.h
            });
            sprite.dst_rect = new SDL_FRect(SDL_FRect{
                (float)dst_rect.x,
                (float)dst_rect.y,
                (float)dst_rect.w,
                (float)dst_rect.h
            });
            sprite.y = depth_offset;

            game->push_sprite(sprite.tex_id, sprite.texture, sprite.src_rect, sprite.dst_rect, sprite.y);
            this->sprites.push_back(sprite);
        } break;

        case MSG_AUDIO: {
            int gain_i, x, y;
            float gain;

            if (4 != sscanf(arr + 1, "%1023s %d,%d,%d", buff, &gain_i, &x, &y)) FATAL_ERROR
            gain = (float)gain_i / 10.0f;

            play_audio(std::string(buff), gain, (float)x, (float)y);
        } break;
        }
    }

    this->just_loaded_map = false;
}

bool NetReceiver::handle_important_message(const char *arr) {
    char buff[1024];

    switch (arr[0]) {
    case MSG_SYNC: {
        if (this->just_pushed_feta) {
            sscanf(arr, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            this->skipped_messages += std::string(buff) + '\n';
        } else if (feta != nullptr) {
            int animation;
            char x_str[256], y_str[256];
            if (4 != sscanf(arr + 1, "%255[^,],%255[^,],%d %1023[^\n]\n", x_str, y_str, &animation, buff)) FATAL_ERROR
            x_str[255] = y_str[255] = buff[1023] = '\0';

            feta->x = str_to_float(x_str);
            feta->y = str_to_float(y_str);
            feta->set_animation(animation);
            feta->set_items(std::string(buff));
        } else {
            sscanf(arr + 1, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            game->push_object(FETA_OBJ, std::string(buff));
            this->just_pushed_feta = true;
        }
    } break;

    case MSG_ATTACK:
        if (feta != nullptr) {
            feta->attack(std::atoi(arr + 1));
        } else if (this->just_pushed_feta) {
            sscanf(arr, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            this->skipped_messages += std::string(buff) + '\n';
        }

        break;

    case MSG_PUSH_ITEM:
        if (feta != nullptr) {
            sscanf(arr + 1, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            feta->push_item(std::string(buff));
        } else if (this->just_pushed_feta) {
            sscanf(arr, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            this->skipped_messages += std::string(buff) + '\n';
        }

        break;

    case MSG_PUSH_CHEESE:
        if (feta != nullptr) {
            feta->push_cheese(std::atoi(arr + 1));
        } else if (this->just_pushed_feta) {
            sscanf(arr, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            this->skipped_messages += std::string(buff) + '\n';
        }

        break;

    case MSG_FORCE_DANCE:
        if (feta != nullptr) {
            feta->force_dance(std::atoi(arr + 1));
        } else if (this->just_pushed_feta) {
            sscanf(arr, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            this->skipped_messages += std::string(buff) + '\n';
        }

        break;

    case MSG_DID_HIT:
        if (feta != nullptr) {
            feta->did_hit = true;
        } else if (this->just_pushed_feta) {
            sscanf(arr, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            this->skipped_messages += std::string(buff) + '\n';
        }

        break;

    case MSG_THROW:
        if (feta != nullptr) {
            int throw_x, throw_y;
            if (2 != sscanf(arr + 1, "%d,%d", &throw_x, &throw_y)) FATAL_ERROR
            feta->set_throw(throw_x, throw_y);
        } else if (this->just_pushed_feta) {
            sscanf(arr, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            this->skipped_messages += std::string(buff) + '\n';
        }

        break;

    case MSG_MAX_SPEED:
        if (feta != nullptr) {
            feta->set_max_mov_speed(std::atoi(arr + 1));
        } else if (this->just_pushed_feta) {
            sscanf(arr, "%1023[^\n]\n", buff);
            buff[1023] = '\0';
            this->skipped_messages += std::string(buff) + '\n';
        }

        break;

    case MSG_COLLISION:
        int coord, collider;
        if (2 != sscanf(arr + 1, "%d,%d", &coord, &collider)) FATAL_ERROR

        if (coord >= 0 && coord < game->cols * game->rows &&
            collider >= 0 && collider < n_MapColliders) {
            game->collision[coord] = collider;
        }

        break;

    default:
        return false;
    }

    return true;
}

void NetReceiver::load_map(const std::string &map, const char *remaining_msgs) {
    if (remaining_msgs != NULL && *remaining_msgs != '\0') {
        this->skipped_messages += remaining_msgs;
    }

    for (Game::SpriteRender &sprite : this->sprites) {
        delete sprite.src_rect;
        delete sprite.dst_rect;
    }
    this->sprites.clear();

    game->map = map;

    this->just_loaded_map = true;
}