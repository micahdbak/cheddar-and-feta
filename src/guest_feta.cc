#include "guest_feta.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "audio/audio.h"
#include "game_hud.h"
#include "guest_item.h"
#include "guest_peer_sender.h"
#include "input/controller.h"
#include "input/input.h"
#include "mouse.h"
#include "object.h"
#include "peer/peer.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/macros.h"
#include "utils/utils.h"

GuestFeta* guest_feta = nullptr;

GuestFeta::GuestFeta(float x, float y, int animation,
                     std::vector<GameHud::Item> items) {
  if (guest_feta != nullptr) {
    // TODO(micahdbak): error
    std::exit(1);
  }
  guest_feta = this;

  this->x = x;
  this->y = y;

  this->sprite = new thoom::Sprite("sprites/feta.bmp", 32, 32, 250);
  this->sprite->set_animation(animation);
  this->emotes = new thoom::Sprite("sprites/emotes.bmp", 32, 32, 0);

  this->items = items;

  thoom::Renderer::instance->set_view(this->x, this->y);

  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;
  this->emote_rect.w = 32.0f;
  this->emote_rect.h = 32.0f;
}

GuestFeta::~GuestFeta() {
  delete this->sprite;
  delete this->emotes;
  guest_feta = nullptr;
}

void GuestFeta::step() {
  thoom::Audio::instance->set_listener(this->x, this->y);  // for audio

  // ---- items ----

  if (!this->items.empty()) {
    if (thoom::Input::instance->world.consume_hit(thoom::Button::DIGIT)) {
      // -1 reserved for no item; 0+ for indexing into this->items
      // (note that the first number key is 1, not 0, hence subtract 2)
      int new_item = thoom::Input::instance->world.digit - 2;

      if (new_item >= -1 && new_item < (int)this->items.size()) {
        this->sel_item = new_item;
      }
    }
  } else {
    // no item when no items (duh)
    this->sel_item = -1;
  }

  std::string sel_item_id;
  int sel_item_count;

  if (this->sel_item == -1) {
    sel_item_id = ITEM_NONE;
    sel_item_count = 1;
  } else {
    sel_item_id = this->items[this->sel_item].item_id;
    sel_item_count = this->items[this->sel_item].count;
  }

  // ---- state management ----

  float mov_speed = 0.0f;
  int x_dir = 0, y_dir = 0;

  int dancing_animation = thoom::Input::instance->world.cheat_code(1, 2, 3, 4)
                              ? DANCING2_ANIMATION
                              : DANCING_ANIMATION;

  switch (this->is_busy) {
      // ---- not busy ----

    case FALSE:
      // dancing
      if (thoom::Input::instance->world.down[thoom::Button::DANCE]) {
        this->sprite->set_animation(dancing_animation);
        this->sprite->update_frame();
        this->sprite->interval_ms = 200;
        break;  // don't do anything but dance
      }

      // when no longer dancing, face down
      if (this->sprite->animation == dancing_animation) {
        this->sprite->set_animation(0);
      }

      // move using arrow keys
      x_dir = thoom::Input::instance->world.held_dx();
      y_dir = thoom::Input::instance->world.held_dy();

      if (x_dir != 0 || y_dir != 0) {
        this->sprite->set_animation(thoom::direction_from_dirs(x_dir, y_dir));

        if (this->sprite->update_frame() && (this->sprite->frame_i % 2) == 1) {
          thoom::Audio::instance->play("sfx/step.wav", 0.5, this->x, this->y,
                                       true);
          GuestPeerSender::send_audio("sfx/step.wav", 0.5, this->x, this->y);
        }

        mov_speed = this->max_mov_speed;
        this->sprite->interval_ms = 100;
      } else {
        mov_speed = 0.0f;
        thoom::dirs_from_direction(this->sprite->animation % 8, &x_dir, &y_dir);
        this->sprite->set_frame(0);
      }

      // handle an item which changes top speed
      if (guest_item_info[sel_item_id].type == HELD_EFFECT) {
        mov_speed *= guest_item_info[sel_item_id].speed;
      }

      if (thoom::Input::instance->world.consume_hit(thoom::Button::ATTACK)) {
        this->busy_ticks = thoom::Scene::instance->ticks;

        switch (guest_item_info[sel_item_id].type) {
            // ---- attack / use item ----

          case USEFUL:
          case THROWABLE: {
            char buff[256];
            snprintf(buff, sizeof(buff), "%d,%d,%d,%d", (int)this->x,
                     (int)this->y, x_dir, y_dir);
            GuestPeerSender::send_message(MSG_USE_ITEM,
                                          sel_item_id + " " + buff);
            this->remove_item(sel_item_id);

            if (guest_item_info[sel_item_id].type == THROWABLE) {
              this->is_busy = THROWING;

              // set animation to throwing
              this->sprite->set_animation((this->sprite->animation % 8) +
                                          THROWING_ANIMATION);
              this->sprite->set_frame(0);
              this->sprite->interval_ms = 125;

              thoom::Audio::instance->play("sfx/throw.wav", 1.0f, this->x,
                                           this->y, true);
              GuestPeerSender::send_audio("sfx/throw.wav", 1.0, this->x,
                                          this->y);
            }
          } break;

            // ---- eat cheese ----

          case EDIBLE:
            if (this->health < this->max_health) {
              this->health++;
              this->remove_item(sel_item_id);
              GuestPeerSender::send_message(MSG_EAT_CHEESE, std::string());

              this->is_busy = EATING;
              this->busy_ticks = thoom::Scene::instance->ticks;

              // set animation to eating cheese
              this->sprite->set_animation(EATING_ANIMATION);
              this->sprite->interval_ms = 75;

              thoom::Audio::instance->play("sfx/cheese.wav", 1.0f, this->x,
                                           this->y, true);
              GuestPeerSender::send_audio("sfx/cheese.wav", 1.0, this->x,
                                          this->y);
            } else {
              thoom::Audio::instance->play("sfx/full.wav", 1.0f, this->x,
                                           this->y, true);
              GuestPeerSender::send_audio("sfx/full.wav", 1.0, this->x,
                                          this->y);
            }

            break;

            // ---- held effect (*no item*, armour, boots) ----

          case HELD_EFFECT: {
            // if item doesn't deal damage, do nothing
            if (guest_item_info[sel_item_id].damage < 1) {
              break;
            }

            this->is_busy = ATTACKING;

            char buff[256];
            snprintf(buff, sizeof(buff), "%s %d,%d", sel_item_id.c_str(), x_dir,
                     y_dir);
            GuestPeerSender::send_message(MSG_PUSH_HITBOX, std::string(buff));

            // set animation to attacking
            this->sprite->set_animation((this->sprite->animation % 8) +
                                        ATTACKING_ANIMATION);

            thoom::Audio::instance->play("sfx/kick.wav", 1.0f, this->x, this->y,
                                         true);
            GuestPeerSender::send_audio("sfx/kick.wav", 1.0, this->x, this->y);
          } break;

          default:
            break;
        }
      } else if (thoom::Input::instance->world.consume_hit(
                     thoom::Button::TOSS) &&
                 sel_item_id != ITEM_NONE) {
        this->busy_ticks = thoom::Scene::instance->ticks;

        if (x_dir == 0 && y_dir == 0)
          thoom::dirs_from_direction(this->sprite->animation % 8, &x_dir,
                                     &y_dir);

        // toss item
        GuestPeerSender::send_message(
            MSG_TOSS_ITEM,
            sel_item_id + " " + UseItem::options(x, y, x_dir, y_dir));
        this->remove_item(sel_item_id);

        this->is_busy = THROWING;
        this->busy_ticks = thoom::Scene::instance->ticks;

        // set animation to throwing
        this->sprite->set_animation((this->sprite->animation % 8) +
                                    THROWING_ANIMATION);
        this->sprite->set_frame(0);
        this->sprite->interval_ms = 125;

        thoom::Audio::instance->play("sfx/throw.wav", 1.0f, this->x, this->y,
                                     true);
        GuestPeerSender::send_audio("sfx/throw.wav", 1.0, this->x, this->y);
      }

      break;

      // ---- attacking ----

    case ATTACKING: {
      // move slower using arrow keys
      x_dir = thoom::Input::instance->world.held_dx();
      y_dir = thoom::Input::instance->world.held_dy();
      mov_speed = this->max_mov_speed / 2.0f;

      // longer cooldown when actually hit enemy
      int cooldown = this->did_hit ? 500 : 250;

      // will return to normal after << 250 or 500 ms >>
      if (thoom::Scene::instance->ticks - this->busy_ticks > cooldown) {
        this->is_busy = FALSE;
        this->did_hit = false;

        // set animation to walking/running
        this->sprite->set_animation(this->sprite->animation % 8);
      }
    } break;

      // ---- attacked ----

    case ATTACKED:
      // move according to the random throw direction set when attacked
      x_dir = this->throw_x;
      y_dir = this->throw_y;
      mov_speed = MOUSE_DEFAULT_SPEED;

      // will return to normal after << 250 ms >>
      if (thoom::Scene::instance->ticks - this->busy_ticks > 250) {
        this->is_busy = FALSE;

        // set animation to walking/running
        this->sprite->set_animation(this->sprite->animation % 8);
      }

      break;

      // ---- throwing ----

    case THROWING:
      // move slower using arrow keys
      x_dir = thoom::Input::instance->world.held_dx();
      y_dir = thoom::Input::instance->world.held_dy();
      mov_speed = this->max_mov_speed / 2.0f;

      this->sprite->interval_ms = 125;
      this->sprite->update_frame();

      // will return to normal after << 500 ms >>
      if (thoom::Scene::instance->ticks - this->busy_ticks > 500) {
        this->is_busy = FALSE;

        // set animation to walking/running
        this->sprite->set_animation(this->sprite->animation % 8);
      }

      break;

      // ---- eating ----

    case EATING:
      this->sprite->interval_ms = 75;
      this->sprite->update_frame();

      // will return to normal after << 750 ms >>
      if (thoom::Scene::instance->ticks - this->busy_ticks > 750) {
        this->is_busy = FALSE;

        // set animation to walking/running
        this->sprite->set_animation(this->sprite->animation % 8);
      }

      break;

      // ---- down ----

    case DOWNED:
      this->sprite->interval_ms = 250;
      this->sprite->update_frame();

      // will return to normal after << 2000 ms >>
      if (thoom::Scene::instance->ticks - this->busy_ticks > 2000) {
        this->is_busy = FALSE;
        this->health = 5;

        // set animation to walking/running
        this->sprite->set_animation(this->sprite->animation % 8);
      }

      break;

      // ---- forced dancing (ending) ----

    case FORCED_DANCE:
      if (this->sprite->animation != dancing_animation) {
        this->sprite->set_animation(dancing_animation);
      }

      this->sprite->interval_ms = 200;
      this->sprite->update_frame();

      // will return to normal after << [this->dance_until] ms >>
      if (thoom::Scene::instance->ticks - this->busy_ticks >
          this->dance_until) {
        this->is_busy = FALSE;
        this->sprite->set_animation(0);
      }

      break;
  }

  // you get 5 seconds after being "downed" before enemies will attack you again
  // (three seconds of movement)
  if (this->is_down &&
      thoom::Scene::instance->ticks - this->is_down_ticks > 5000) {
    this->is_down = false;
    GuestPeerSender::send_message(MSG_IS_DOWN, std::to_string(0));
  }

  // ---- move cheddar ----

  // new coordinates calculated with direction moving, movement speed, and
  // diagonal multiplier (if necessary)
  if (mov_speed > 0.0f && (x_dir != 0 || y_dir != 0)) {
    float dx = float(x_dir) * (y_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) *
               mov_speed * thoom::Scene::instance->delta;
    float dy = float(y_dir) * (x_dir != 0 ? THOOM_INV_SQRT2_F : 1.0f) *
               mov_speed * thoom::Scene::instance->delta;

    // prevents bad delta time movement (imagine a single frame lag spike)
    dx = THOOM_CLAMP(dx, -4.0f, 4.0f);
    dy = THOOM_CLAMP(dy, -4.0f, 4.0f);

    float new_x = this->x + dx;
    float new_y = this->y + dy;

    bool collision_x = false, collision_y = false;

    if (x_dir != 0) {
      collision_x = Mouse::check_collision(new_x, this->y);
      if (!collision_x) this->x = new_x;
    }

    if (y_dir != 0) {
      collision_y = Mouse::check_collision(this->x, new_y);
      if (!collision_y) this->y = new_y;
    }

    // sliding on collision horizontally or vertically
    if (x_dir != 0 && y_dir == 0) {
      if (collision_x) {
        float new_y1 = this->y + fabs(dx);
        float new_y2 = this->y - fabs(dx);

        bool collision_y1 = Mouse::check_collision(this->x, new_y1);
        bool collision_y2 = Mouse::check_collision(this->x, new_y2);

        if (!collision_y1 && collision_y2) {
          this->y = new_y1;
        } else if (collision_y1 && !collision_y2) {
          this->y = new_y2;
        }
      }
    } else if (y_dir != 0 && x_dir == 0) {
      if (collision_y) {
        float new_x1 = this->x + fabs(dy);
        float new_x2 = this->x - fabs(dy);

        bool collision_x1 = Mouse::check_collision(new_x1, this->y);
        bool collision_x2 = Mouse::check_collision(new_x2, this->y);

        if (!collision_x1 && collision_x2) {
          this->x = new_x1;
        } else if (collision_x1 && !collision_x2) {
          this->x = new_x2;
        }
      }
    }
  }

  // ---- emotes ----

  thoom::Controller* world = &thoom::Input::instance->world;
  int emote = world->digit;

  if (emote != -1 && world->cheat_code(9, 8, 7, emote)) {
    this->which_emote = emote;
  } else if (this->which_emote != -1 &&
             !world->cheat_code(9, 8, 7, this->which_emote)) {
    this->which_emote = -1;
  }
}

void GuestFeta::render() {
  GameHud::draw_items(this->items, this->sel_item, this->health,
                      this->max_health);

  thoom::Renderer::instance->set_view(this->x, this->y);

  this->dst_rect.x = float(int(this->x)) - 16.0f;
  this->dst_rect.y = float(int(this->y)) - 24.0f;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 22);

  this->emote_rect.x = this->dst_rect.x;
  this->emote_rect.y = this->dst_rect.y - 11.0f;

  if (this->which_emote != -1) {
    this->emotes->set_frame(this->which_emote % 8);
    thoom::Renderer::instance->push_sprite(
        this->emotes->tex_id, this->emotes->texture, &this->emotes->frame,
        &this->emote_rect, 34);
  }
}

void GuestFeta::attack(int damage) {
  // don't get attacked if was already attacked / down
  if (this->is_busy == ATTACKED || this->is_busy == DOWNED ||
      this->is_busy == FORCED_DANCE)
    return;

  this->is_busy = ATTACKED;
  this->busy_ticks = thoom::Scene::instance->ticks;
  this->sprite->set_animation((this->sprite->animation % 8) +
                              ATTACKED_ANIMATION);

  std::string sel_item_id;

  if (this->sel_item < 0 || this->sel_item >= this->items.size()) {
    sel_item_id = ITEM_NONE;
  } else {
    sel_item_id = this->items[this->sel_item].item_id;
  }

  int armour = guest_item_info[sel_item_id].armour;

  damage -= armour;

  // nothing to do if damage is 0
  if (damage <= 0) {
    return;
  }

  this->health -= damage;
  thoom::Audio::instance->play("sfx/hurt.wav", 1.0f, this->x, this->y, true);
  GuestPeerSender::send_audio("sfx/hurt.wav", 1.0, this->x, this->y);

  // if dead, go down
  if (this->health <= 0) {
    this->health = 0;

    this->is_busy = DOWNED;
    this->busy_ticks = thoom::Scene::instance->ticks;
    this->is_down_ticks = thoom::Scene::instance->ticks;
    this->sprite->set_animation((this->sprite->animation % 8) + DOWN_ANIMATION);

    this->is_down = true;
    GuestPeerSender::send_message(MSG_IS_DOWN, std::to_string(1));
  }
}

void GuestFeta::push_item(const std::string& item_id) {
  if (guest_item_info.find(item_id) == guest_item_info.end()) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  GameHud::Item new_item;
  new_item.item_id = item_id;
  new_item.count = 1;

  if (this->items.empty()) {
    this->items.push_back(new_item);
    return;
  }

  // see if the item is already in inventory, and increment count
  int i = 0;
  for (auto it = this->items.begin(); it != this->items.end(); ++it, ++i) {
    if (it->item_id == item_id) {
      it->count++;
      this->sel_item = i;
      return;
    }
  }

  // item not in inventory, push to back
  this->items.push_back(new_item);
  this->sel_item = i;
}

void GuestFeta::push_cheese(int amount) {
  GameHud::Item new_item;
  new_item.item_id = ITEM_CHEESE;
  new_item.count = amount;

  if (this->items.empty()) {
    this->items.push_back(new_item);
    return;
  }

  // see if already has cheese, and increment by amount
  int i = 0;
  for (auto it = this->items.begin(); it != this->items.end(); ++it, ++i) {
    if (it->item_id == ITEM_CHEESE) {
      it->count += amount;
      return;
    }
  }

  // cheese not in inventory, push to back
  this->items.push_back(new_item);
}

void GuestFeta::remove_item(const std::string& item_id) {
  for (auto it = this->items.begin(); it != this->items.end(); it++) {
    if (it->item_id == item_id && it->count > 0) {
      it->count--;
      if (it->count <= 0) {
        this->items.erase(it);
        this->sel_item = -1;
        break;
      }
    }
  }
}

void GuestFeta::force_dance(uint64_t timeout_ms) {
  this->is_busy = GuestFeta::Busy::FORCED_DANCE;
  this->dance_until = timeout_ms;
}

void GuestFeta::set_animation(int animation) {
  this->sprite->set_animation(animation);
}

void GuestFeta::set_items(const std::string& items_s) {
  this->items = Mouse::read_items(items_s, GuestFeta::known_item);

  if (this->sel_item >= this->items.size()) {
    this->sel_item = this->items.size() - 1;
  }
}

void GuestFeta::set_throw(int throw_x, int throw_y) {
  this->throw_x = throw_x;
  this->throw_y = throw_y;
}

void GuestFeta::set_max_mov_speed(float max_mov_speed) {
  this->max_mov_speed = max_mov_speed;
}

thoom::Object* GuestFeta::create_object(thoom::Scene* scene,
                                        const std::string& options) {
  int animation;
  char x_str[256], y_str[256], items_s[1024];
  int fields = sscanf(options.c_str(), "%255[^,],%255[^,],%d %1023s", x_str,
                      y_str, &animation, items_s);

  if (fields == 3) {
    items_s[0] = '\0';
  } else if (fields != 4) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new GuestFeta(thoom::str_to_float(x_str), thoom::str_to_float(y_str),
                       animation,
                       Mouse::read_items(items_s, GuestFeta::known_item));
}

bool GuestFeta::known_item(const GameHud::Item& item) {
  return guest_item_info.find(item.item_id) != guest_item_info.end() &&
         item.count > 0;
}
