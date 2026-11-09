#include "guest_peer_receiver.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <string>
#include <vector>

#include "audio/audio.h"
#include "guest_feta.h"
#include "icon.h"
#include "map/map_collider.h"
#include "object.h"
#include "peer/peer.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "texture/texture.h"
#include "utils/utils.h"

GuestPeerReceiver::~GuestPeerReceiver() { this->clear_sprites(); }

void GuestPeerReceiver::clear_sprites() {
  for (thoom::SpriteRender& sprite : this->sprites) {
    delete sprite.src_rect;
    delete sprite.dst_rect;
  }

  this->sprites.clear();
}

void GuestPeerReceiver::step() {
  // ensure we are in maps/init if disconnected, and do nothing else
  if (thoom::Peer::instance->get_state() != thoom::Peer::kStateConnected) {
    // The cached frame belongs to the old map and outlives it, so drop it
    // instead of drawing it over the init screen.
    this->clear_sprites();
    this->pending_messages = {};
    this->skipped_messages.clear();

    // if currently in a map other than the init map, reset the peer
    // and go to that
    if (thoom::Scene::instance->map->path != "maps/init") {
      thoom::Peer::instance->set_state(thoom::Peer::kStateTryReset);
      thoom::Scene::instance->request_map("maps/init");
      last_frame_ticks = 0;
    }

    return;
  }

  this->just_pushed_feta = false;

  if (!this->skipped_messages.empty()) {
    std::string skipped_copy = this->skipped_messages;
    this->skipped_messages.clear();

    for (const char* arr = skipped_copy.c_str(); *arr != '\0';
         arr = thoom::next_line(arr)) {
      this->handle_important_message(arr);  // disregards other message types
    }
  }

  // Replay the batch deferred by a map change before this step's frames.
  std::queue<std::string> received =
      thoom::Peer::instance->recv_queue.take_all();

  while (!received.empty()) {
    this->pending_messages.push(received.front());
    received.pop();
  }

  // if empty, reuse last frame
  if (this->pending_messages.empty()) {
    // 5 seconds of no frames; disconnect
    if (thoom::Scene::instance->ticks - last_frame_ticks > 5000 &&
        last_frame_ticks != 0) {
      thoom::Peer::instance->set_state(thoom::Peer::kStateTryReset);
      last_frame_ticks = 0;
    }

    return;
  }

  // set this as we have new frame(s) to parse
  last_frame_ticks = thoom::Scene::instance->ticks;

  this->clear_sprites();

  char buff[1024];

  // read previous feta related messages
  while (this->pending_messages.size() > 1) {
    std::string msg = this->pending_messages.front();
    this->pending_messages.pop();

    for (const char* arr = msg.c_str(); *arr != '\0';
         arr = thoom::next_line(arr)) {
      if (arr[0] == MSG_MAP) {
        sscanf(arr + 1, "%1023[^\n]\n", buff);
        buff[1023] = '\0';

        if (thoom::Scene::instance->map->path != buff) {
          this->load_map(std::string(buff), thoom::next_line(arr));

          // The map load destroys the local Feta. The rest of this batch may
          // hold its sync, or another map change and its sync, so keep it
          // pending and replay it after the load.
          return;  // don't read rest of messages
        }
      }

      this->handle_important_message(arr);  // disregards other message types
    }
  }

  // most recent message
  std::string msg = this->pending_messages.front();
  this->pending_messages.pop();

  for (const char* arr = msg.c_str(); *arr != '\0';
       arr = thoom::next_line(arr)) {
    if (this->handle_important_message(arr)) {
      continue;
    }

    switch (arr[0]) {
      case MSG_MAP:
        sscanf(arr + 1, "%1023[^\n]\n", buff);
        buff[1023] = '\0';

        if (thoom::Scene::instance->map->path != buff) {
          this->load_map(std::string(buff), thoom::next_line(arr));
          return;  // don't read rest of messages
        }

        break;

      case MSG_SPRITE: {
        if (this->just_loaded_map) {
          break;
        }

        SDL_Rect src_rect, dst_rect;
        int depth_offset;

        if (10 != sscanf(arr + 1, "%1023s %d,%d,%d,%d %d,%d,%d,%d %d", buff,
                         &src_rect.x, &src_rect.y, &src_rect.w, &src_rect.h,
                         &dst_rect.x, &dst_rect.y, &dst_rect.w, &dst_rect.h,
                         &depth_offset)) {
          // TODO(micahdbak): error
          std::exit(1);
        }
        buff[1023] = '\0';

        thoom::SpriteRender sprite;
        sprite.tex_id = buff;  // buff is tex_id and sprite path

        // don't double render feta
        if (sprite.tex_id == "sprites/feta.bmp") {
          continue;
        }

        // Icons are embedded in shared, so they are not on disk.
        if (sprite.tex_id == "sprites/icons.bmp") {
          sprite.texture = icons;
        } else {
          sprite.texture = thoom::Texture::instance->load(sprite.tex_id);
        }

        if (sprite.texture == nullptr) {
          continue;
        }

        sprite.src_rect =
            new SDL_FRect(SDL_FRect{(float)src_rect.x, (float)src_rect.y,
                                    (float)src_rect.w, (float)src_rect.h});
        sprite.dst_rect =
            new SDL_FRect(SDL_FRect{(float)dst_rect.x, (float)dst_rect.y,
                                    (float)dst_rect.w, (float)dst_rect.h});
        sprite.y = depth_offset;

        this->sprites.push_back(sprite);
      } break;

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
    }
  }

  this->just_loaded_map = false;
}

void GuestPeerReceiver::render() {
  for (thoom::SpriteRender& sprite : this->sprites) {
    thoom::Renderer::instance->push_sprite(sprite.tex_id, sprite.texture,
                                           sprite.src_rect, sprite.dst_rect,
                                           sprite.y);
  }
}

bool GuestPeerReceiver::handle_important_message(const char* arr) {
  char buff[1024];

  switch (arr[0]) {
    case MSG_SYNC: {
      char map[1024], x_str[256], y_str[256], line[1024];
      int animation;

      int fields = sscanf(arr + 1, "%1023s %255[^,],%255[^,],%d %1023[^\n]\n",
                          map, x_str, y_str, &animation, buff);
      if (fields < 4) {
        // TODO(micahdbak): error
        std::exit(1);
      }
      map[1023] = x_str[255] = y_str[255] = buff[1023] = '\0';
      if (fields == 4) buff[0] = '\0';

      // The map message and its sync can arrive in different steps. Replay
      // the sync once the guest loads the map it was sent for.
      if (thoom::Scene::instance->map->path != map) {
        sscanf(arr, "%1023[^\n]\n", line);
        line[1023] = '\0';
        this->skipped_messages += std::string(line) + '\n';
        break;
      }

      if (this->just_pushed_feta) {
        sscanf(arr, "%1023[^\n]\n", line);
        line[1023] = '\0';
        this->skipped_messages += std::string(line) + '\n';
      } else if (guest_feta != nullptr) {
        guest_feta->x = thoom::str_to_float(x_str);
        guest_feta->y = thoom::str_to_float(y_str);
        guest_feta->set_animation(animation);
        guest_feta->set_items(std::string(buff));
      } else {
        snprintf(line, sizeof(line), "%s,%s,%d %s", x_str, y_str, animation,
                 buff);
        thoom::Scene::instance->create_object(FETA_OBJ, std::string(line));
        this->just_pushed_feta = true;
      }
    } break;

    case MSG_ATTACK:
      if (guest_feta != nullptr) {
        guest_feta->attack(std::atoi(arr + 1));
      } else if (this->just_pushed_feta) {
        sscanf(arr, "%1023[^\n]\n", buff);
        buff[1023] = '\0';
        this->skipped_messages += std::string(buff) + '\n';
      }

      break;

    case MSG_PUSH_ITEM:
      if (guest_feta != nullptr) {
        sscanf(arr + 1, "%1023[^\n]\n", buff);
        buff[1023] = '\0';
        guest_feta->push_item(std::string(buff));
      } else if (this->just_pushed_feta) {
        sscanf(arr, "%1023[^\n]\n", buff);
        buff[1023] = '\0';
        this->skipped_messages += std::string(buff) + '\n';
      }

      break;

    case MSG_PUSH_CHEESE:
      if (guest_feta != nullptr) {
        guest_feta->push_cheese(std::atoi(arr + 1));
      } else if (this->just_pushed_feta) {
        sscanf(arr, "%1023[^\n]\n", buff);
        buff[1023] = '\0';
        this->skipped_messages += std::string(buff) + '\n';
      }

      break;

    case MSG_FORCE_DANCE:
      if (guest_feta != nullptr) {
        guest_feta->force_dance(std::atoi(arr + 1));
      } else if (this->just_pushed_feta) {
        sscanf(arr, "%1023[^\n]\n", buff);
        buff[1023] = '\0';
        this->skipped_messages += std::string(buff) + '\n';
      }

      break;

    case MSG_DID_HIT:
      if (guest_feta != nullptr) {
        guest_feta->did_hit = true;
      } else if (this->just_pushed_feta) {
        sscanf(arr, "%1023[^\n]\n", buff);
        buff[1023] = '\0';
        this->skipped_messages += std::string(buff) + '\n';
      }

      break;

    case MSG_THROW:
      if (guest_feta != nullptr) {
        int throw_x, throw_y;
        if (2 != sscanf(arr + 1, "%d,%d", &throw_x, &throw_y)) {
          // TODO(micahdbak): error
          std::exit(1);
        }
        guest_feta->set_throw(throw_x, throw_y);
      } else if (this->just_pushed_feta) {
        sscanf(arr, "%1023[^\n]\n", buff);
        buff[1023] = '\0';
        this->skipped_messages += std::string(buff) + '\n';
      }

      break;

    case MSG_MAX_SPEED:
      if (guest_feta != nullptr) {
        guest_feta->set_max_mov_speed(std::atoi(arr + 1));
      } else if (this->just_pushed_feta) {
        sscanf(arr, "%1023[^\n]\n", buff);
        buff[1023] = '\0';
        this->skipped_messages += std::string(buff) + '\n';
      }

      break;

    case MSG_COLLISION:
      int coord, collider;
      if (2 != sscanf(arr + 1, "%d,%d", &coord, &collider)) {
        // TODO(micahdbak): error
        std::exit(1);
      }

      if (coord >= 0 &&
          coord < thoom::Scene::instance->map->cols *
                      thoom::Scene::instance->map->rows &&
          collider >= 0 && collider < thoom::n_MapColliders) {
        int x = coord % thoom::Scene::instance->map->cols;
        int y = coord / thoom::Scene::instance->map->cols;
        thoom::Scene::instance->map->collider->set(x, y, collider);
      }

      break;

    default:
      return false;
  }

  return true;
}

void GuestPeerReceiver::load_map(const std::string& map,
                                 const char* remaining_msgs) {
  if (remaining_msgs != NULL && *remaining_msgs != '\0') {
    this->skipped_messages += remaining_msgs;
  }

  this->clear_sprites();

  thoom::Scene::instance->request_map(map, false);

  this->just_loaded_map = true;
}

thoom::Object* GuestPeerReceiver::create_object(thoom::Scene* scene,
                                                const std::string& options) {
  return new GuestPeerReceiver();
}
