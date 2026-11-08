#include "cheddar.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "audio/audio.h"
#include "foes/foe.h"
#include "game_hud.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "input/controller.h"
#include "input/input.h"
#include "items/cheese.h"
#include "items/item.h"
#include "items/persister.h"
#include "items/save.h"
#include "items/tossed.h"
#include "mouse.h"
#include "object.h"
#include "peer/peer.h"
#include "remote_feta.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/macros.h"
#include "utils/utils.h"

Cheddar::Cheddar(std::vector<Mouse::SpawnCoord>& coordinates,
                 std::string options) {
  if (cheddar != nullptr) {
    // TODO(micahdbak): error
    std::exit(1);
  }
  cheddar = this;
  this->is_feta = false;

  this->sprite = new thoom::Sprite("sprites/cheddar.bmp", 32, 32, 250);
  this->emotes = new thoom::Sprite("sprites/emotes.bmp", 32, 32, 0);

  std::string items_s = thoom::save.value(CHEDDAR_OBJ MOUSE_ITEMS);
  if (!items_s.empty()) {
    this->items = Mouse::read_items(items_s);
  } else {
    this->items.push_back(GameHud::Item{ITEM_SAVE, 1});
  }

  int feta_x = 0, feta_y = 0, feta_animation = 0;

  // if loading a save, read the location from the save file
  if (thoom::save.geti(LOAD_SAVE) && thoom::save.has(CHEDDAR_OBJ MOUSE_X)) {
    this->x = thoom::save.getf(CHEDDAR_OBJ MOUSE_X);
    this->y = thoom::save.getf(CHEDDAR_OBJ MOUSE_Y);
    this->sprite->set_animation(thoom::save.geti(CHEDDAR_OBJ MOUSE_ANIMATION));

    // don't set remote feta options; it loads from the save file as well
  } else {
    // get which coordinate to spawn at
    int coord = thoom::save.geti(MOUSE_SPAWN_AT);
    thoom::save.puti(MOUSE_SPAWN_AT, 0);  // unset

    // validate
    if (coordinates.empty() || coord < 0 || coord >= coordinates.size()) {
      // TODO(micahdbak): error
      std::exit(1);
    }

    this->x = coordinates[coord].x;
    this->y = coordinates[coord].y;
    this->sprite->set_animation(coordinates[coord].animation);

    feta_x = coordinates[coord].x;
    feta_y = coordinates[coord].y;
    feta_animation = coordinates[coord].animation;
  }

  this->tile_x = (int)this->x / thoom::Scene::instance->map->tile_width;
  this->tile_y = (int)this->y / thoom::Scene::instance->map->tile_height;
  foe_path_find(this);

  thoom::Renderer::instance->set_view(this->x, this->y);

  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;
  this->emote_rect.w = 32.0f;
  this->emote_rect.h = 32.0f;

  // make remote feta
  char feta_options[256];
  snprintf(feta_options, sizeof(feta_options), "%d,%d,%d", feta_x, feta_y,
           feta_animation);
  thoom::Scene::instance->create_object(FETA_OBJ, std::string(feta_options));

  // push item persister and hurtbox
  thoom::Scene::instance->create_object(ITEM_PERSISTER_OBJ, "");
  thoom::Scene::instance->create_object(
      HURTBOX_OBJ, HurtBox::options(this->id, -8, -8, 16, 12));

  this->is_down = false;
}

Cheddar::~Cheddar() {
  delete this->sprite;
  delete this->emotes;
  cheddar = nullptr;
}

void Cheddar::step() {
  if (mice_locked) {
    thoom::Renderer::instance->push_sprite(
        this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
        &this->dst_rect, 22);
    return;
  }

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

  GameHud::draw_items(this->items, this->sel_item, this->health,
                      this->max_health);

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
                                       false);
        }

        mov_speed = this->max_mov_speed;
        this->sprite->interval_ms = 100;
      } else {
        mov_speed = 0.0f;
        thoom::dirs_from_direction(this->sprite->animation % 8, &x_dir, &y_dir);
        this->sprite->set_frame(0);
      }

      // handle an item which changes top speed
      if (item_info[sel_item_id].type == HELD_EFFECT) {
        mov_speed *= item_info[sel_item_id].speed;
      }

      if (thoom::Input::instance->world.consume_hit(thoom::Button::ATTACK)) {
        this->busy_ticks = thoom::Scene::instance->ticks;

        switch (item_info[sel_item_id].type) {
            // ---- attack / use item ----

          case USEFUL:
          case THROWABLE:
            char options[256];
            snprintf(options, sizeof(options), "%d,%d,%d,%d,%d", int(this->x),
                     int(this->y), x_dir, y_dir, this->id);
            thoom::Scene::instance->create_object(sel_item_id + USE_OBJ,
                                                  std::string(options));
            this->remove_item(sel_item_id);

            if (item_info[sel_item_id].type == THROWABLE) {
              this->is_busy = THROWING;

              // set animation to throwing
              this->sprite->set_animation((this->sprite->animation % 8) +
                                          THROWING_ANIMATION);
              this->sprite->set_frame(0);
              this->sprite->interval_ms = 125;

              thoom::Audio::instance->play("sfx/throw.wav", 1.0f, this->x,
                                           this->y, false);
            }

            break;

            // ---- eat cheese ----

          case EDIBLE:
            if (this->health < this->max_health) {
              this->health++;
              this->remove_item(sel_item_id);

              this->is_busy = EATING;
              this->busy_ticks = thoom::Scene::instance->ticks;

              // set animation to eating cheese
              this->sprite->set_animation(EATING_ANIMATION);
              this->sprite->interval_ms = 75;

              thoom::Audio::instance->play("sfx/cheese.wav", 1.0f, this->x,
                                           this->y, false);
            } else {
              thoom::Audio::instance->play("sfx/full.wav", 1.0f, this->x,
                                           this->y, false);
            }

            break;

            // ---- held effect (*no item*, armour, boots) ----

          case HELD_EFFECT: {
            // if item doesn't deal damage, do nothing
            if (item_info[sel_item_id].damage < 1) {
              break;
            }

            this->is_busy = ATTACKING;
            int x_off, y_off;
            HitBox::make_offset(x_dir, y_dir, &x_off, &y_off, 8.0f);
            HitBox::Properties props = {item_info[sel_item_id].damage, 200,
                                        100};
            props.single_use = true;
            thoom::Scene::instance->create_object(
                HITBOX_OBJ, HitBox::options(this->id, this->id, x_off - 16,
                                            y_off - 18, 32, 32, props));

            // set animation to attacking
            this->sprite->set_animation((this->sprite->animation % 8) +
                                        ATTACKING_ANIMATION);

            thoom::Audio::instance->play("sfx/kick.wav", 1.0f, this->x, this->y,
                                         false);
          } break;

          default:
            break;
        }
      } else if (thoom::Input::instance->world.consume_hit(
                     thoom::Button::TOSS) &&
                 sel_item_id != ITEM_NONE && sel_item_id != ITEM_SAVE) {
        this->busy_ticks = thoom::Scene::instance->ticks;

        if (x_dir == 0 && y_dir == 0)
          thoom::dirs_from_direction(this->sprite->animation % 8, &x_dir,
                                     &y_dir);

        // toss item
        thoom::Scene::instance->create_object(
            TOSSED_ITEM_OBJ, TossedItem::options(this->x, this->y, x_dir, y_dir,
                                                 false, sel_item_id));
        this->remove_item(sel_item_id);

        this->is_busy = THROWING;
        this->busy_ticks = thoom::Scene::instance->ticks;

        // set animation to throwing
        this->sprite->set_animation((this->sprite->animation % 8) +
                                    THROWING_ANIMATION);
        this->sprite->set_frame(0);
        this->sprite->interval_ms = 125;

        thoom::Audio::instance->play("sfx/throw.wav", 1.0f, this->x, this->y,
                                     false);
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

    // if moved onto a new tile, update the path finding
    int new_tile_x = (int)this->x / thoom::Scene::instance->map->tile_width;
    int new_tile_y = (int)this->y / thoom::Scene::instance->map->tile_height;
    if (new_tile_x != this->tile_x || new_tile_y != this->tile_y) {
      foe_path_find(this);
      this->tile_x = new_tile_x;
      this->tile_y = new_tile_y;
    }
  }

  // ---- display ----

#ifdef ONSCREEN_DEBUG
  for (int ty = this->tile_y - 12; ty < this->tile_y + 13; ty++) {
    for (int tx = this->tile_x - 12; tx < this->tile_x + 13; tx++) {
      foe_debug_tile(tx, ty);
    }
  }
#endif

  thoom::Renderer::instance->set_view(this->x, this->y);
  this->dst_rect.x = float(int(this->x)) - 16.0f;
  this->dst_rect.y = float(int(this->y)) - 24.0f;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 22);

  thoom::Controller* world = &thoom::Input::instance->world;
  int emote = world->digit;

  if (emote != -1 && world->cheat_code(9, 8, 7, emote)) {
    this->which_emote = emote;
  } else if (this->which_emote != -1 &&
             !world->cheat_code(9, 8, 7, this->which_emote)) {
    this->which_emote = -1;
  }

  this->emote_rect.x = this->dst_rect.x;
  this->emote_rect.y = this->dst_rect.y - 11.0f;

  if (this->which_emote != -1) {
    this->emotes->set_frame(this->which_emote % 8);
    thoom::Renderer::instance->push_sprite(
        this->emotes->tex_id, this->emotes->texture, &this->emotes->frame,
        &this->emote_rect, 34);
  }
}

void Cheddar::save_data() {
  thoom::save.putf(CHEDDAR_OBJ MOUSE_X, this->x);
  thoom::save.putf(CHEDDAR_OBJ MOUSE_Y, this->y);
  thoom::save.puti(CHEDDAR_OBJ MOUSE_ANIMATION, this->sprite->animation);
  thoom::save.data[CHEDDAR_OBJ MOUSE_ITEMS] = Mouse::encode_items(this->items);
}

void Cheddar::attack(int damage) {
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

  int armour = item_info[sel_item_id].armour;

  damage -= armour;

  // nothing to do if damage is 0
  if (damage <= 0) {
    return;
  }

  this->health -= damage;
  thoom::Audio::instance->play("sfx/hurt.wav", 1.0f, this->x, this->y, false);

  // if dead, go down
  if (this->health <= 0) {
    this->health = 0;

    this->is_busy = DOWNED;
    this->busy_ticks = thoom::Scene::instance->ticks;
    this->is_down_ticks = thoom::Scene::instance->ticks;
    this->sprite->set_animation((this->sprite->animation % 8) + DOWN_ANIMATION);

    this->is_down = true;

    uint64_t peer_state = thoom::Peer::instance->state.load();

    // game over if the remote feta is down or disconnected
    if ((remote_feta != nullptr && remote_feta->is_down) ||
        peer_state == thoom::Peer::kStateNoConnection ||
        peer_state == thoom::Peer::kStateWaitingForPeer) {
      thoom::Scene::instance->request_map("maps/dead");
    }
  }
}

void Cheddar::push_item(const std::string& item_id) {
  if (item_info.find(item_id) == item_info.end()) {
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

void Cheddar::push_cheese(int amount) {
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

void Cheddar::remove_item(const std::string& item_id) {
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

void Cheddar::force_dance(Uint64 timeout_ms) {
  this->is_busy = Cheddar::Busy::FORCED_DANCE;
  this->dance_until = timeout_ms;
}

void Cheddar::set_throw(int throw_x, int throw_y) {
  this->throw_x = throw_x;
  this->throw_y = throw_y;
}

void Cheddar::set_max_mov_speed(float max_mov_speed) {
  this->max_mov_speed = max_mov_speed;
}

void Cheddar::signal_down() {
  // signal implies that the remote feta is down; check if we are down too
  if (this->is_down && this->is_busy == DOWNED && remote_feta != nullptr &&
      remote_feta->is_down) {
    thoom::Scene::instance->request_map("maps/dead");
  }
}

thoom::Object* Cheddar::create_object(thoom::Scene* scene,
                                      const std::string& options) {
  const char* arr = options.c_str();
  std::vector<Mouse::SpawnCoord> coordinates;

  int i = 0;
  do {
    int x = 0, y = 0, animation = 0;
    if (3 != sscanf(arr, "%d,%d,%d", &x, &y, &animation)) break;

    Mouse::SpawnCoord coord;
    coord.x = (float)x;
    coord.y = (float)y;
    coord.animation = animation;
    coordinates.push_back(coord);

    while (*arr != '\0' && *arr != ' ') arr++;
    if (*arr == '\0') break;
    while (*++arr == ' ') arr++;
    if (*arr == '\0') break;
  } while (++i < 10);  // max ten coords, incase something really breaks

  return new Cheddar(coordinates, options);
}
