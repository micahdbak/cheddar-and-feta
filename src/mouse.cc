#include "mouse.h"

#include <cfloat>
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
#include "input/controller.h"
#include "input/input.h"
#include "items/cheese.h"
#include "items/item.h"
#include "items/save.h"
#include "items/tossed.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "thoom/macros.h"
#include "utils/utils.h"

IMouse* cheddar = nullptr;
IMouse* remote_feta = nullptr;

IMouse* IMouse::closest_mouse(float x, float y, float min_distance,
                              bool forced) {
  float distance_cheddar =
      (cheddar->is_down && !forced)
          ? FLT_MAX
          : thoom::distance_between_points(x, y, cheddar->x, cheddar->y);
  float distance_feta = (remote_feta->is_down && !forced)
                            ? FLT_MAX
                            : thoom::distance_between_points(
                                  x, y, remote_feta->x, remote_feta->y);

  if (distance_cheddar < distance_feta) {
    if (min_distance < 0.1f || distance_cheddar < min_distance) {
      return cheddar;
    }
  } else if (min_distance < 0.1f || distance_feta < min_distance) {
    return remote_feta;
  }

  return nullptr;  // no close-enough mouse
}

Mouse::Mouse(const char* sprite_path, bool is_feta, const char* save_prefix)
    : save_id(save_prefix) {
  this->is_feta = is_feta;

  this->sprite = new thoom::Sprite(sprite_path, 32, 32, 250);
  this->emotes = new thoom::Sprite("sprites/emotes.bmp", 32, 32, 0);

  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;
  this->emote_rect.w = 32.0f;
  this->emote_rect.h = 32.0f;
}

Mouse::~Mouse() {
  delete this->sprite;
  delete this->emotes;
}

void Mouse::step() {
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

  std::string sel_item_id = this->selected_item();

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
                                       this->is_feta);
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
          case THROWABLE: {
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
                                           this->y, this->is_feta);
            }

            break;
          }

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
                                           this->y, this->is_feta);
            } else {
              thoom::Audio::instance->play("sfx/full.wav", 1.0f, this->x,
                                           this->y, this->is_feta);
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
                                         this->is_feta);
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
                                     this->is_feta);
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

  this->move(x_dir, y_dir, mov_speed);

  // ---- display ----

#ifdef ONSCREEN_DEBUG
  for (int ty = this->tile_y - 12; ty < this->tile_y + 13; ty++) {
    for (int tx = this->tile_x - 12; tx < this->tile_x + 13; tx++) {
      foe_debug_tile(tx, ty);
    }
  }
#endif

  thoom::Controller* world = &thoom::Input::instance->world;
  int emote = world->digit;

  if (emote != -1 && world->cheat_code(9, 8, 7, emote)) {
    this->which_emote = emote;
  } else if (this->which_emote != -1 &&
             !world->cheat_code(9, 8, 7, this->which_emote)) {
    this->which_emote = -1;
  }

  thoom::Renderer::instance->set_view(this->x, this->y);
  this->draw();
}

void Mouse::save_data() {
  thoom::save.putf(this->save_id + MOUSE_X, this->x);
  thoom::save.putf(this->save_id + MOUSE_Y, this->y);
  thoom::save.puti(this->save_id + MOUSE_ANIMATION, this->sprite->animation);
  thoom::save.data[this->save_id + MOUSE_ITEMS] =
      Mouse::encode_items(this->items);
}

void Mouse::attack(int damage) {
  // don't get attacked if was already attacked / down
  if (this->is_busy == ATTACKED || this->is_busy == DOWNED ||
      this->is_busy == FORCED_DANCE)
    return;

  this->is_busy = ATTACKED;
  this->busy_ticks = thoom::Scene::instance->ticks;
  this->sprite->set_animation((this->sprite->animation % 8) +
                              ATTACKED_ANIMATION);

  int armour = item_info[this->selected_item()].armour;

  damage -= armour;

  // nothing to do if damage is 0
  if (damage <= 0) {
    return;
  }

  this->health -= damage;
  thoom::Audio::instance->play("sfx/hurt.wav", 1.0f, this->x, this->y,
                               this->is_feta);

  // if dead, go down
  if (this->health <= 0) {
    this->health = 0;

    this->is_busy = DOWNED;
    this->busy_ticks = thoom::Scene::instance->ticks;
    this->is_down_ticks = thoom::Scene::instance->ticks;
    this->sprite->set_animation((this->sprite->animation % 8) + DOWN_ANIMATION);

    this->is_down = true;
    this->on_down();
  }
}

void Mouse::push_item(const std::string& item_id) {
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

void Mouse::push_cheese(int amount) {
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

void Mouse::remove_item(const std::string& item_id) {
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

void Mouse::force_dance(uint64_t timeout_ms) {
  this->is_busy = Mouse::Busy::FORCED_DANCE;
  this->dance_until = timeout_ms;
}

void Mouse::set_throw(int throw_x, int throw_y) {
  this->throw_x = throw_x;
  this->throw_y = throw_y;
}

void Mouse::set_max_mov_speed(float max_mov_speed) {
  this->max_mov_speed = max_mov_speed;
}

std::string Mouse::encode_items(const std::vector<GameHud::Item>& items) {
  std::string items_s = "";
  for (int i = 0; i < items.size(); i++) {
    if (items[i].count <= 0) continue;

    char item[256];
    snprintf(item, sizeof(item), "%s*%d", items[i].item_id.c_str(),
             items[i].count);
    items_s += item;

    if (i + 1 < items.size()) {
      items_s += ",";
    }
  }
  return items_s;
}

std::vector<GameHud::Item> Mouse::read_items(const std::string& items_s,
                                             ItemFilter keep) {
  std::vector<GameHud::Item> ret;

  if (!items_s.empty()) {
    const char* arr = items_s.c_str();
    int i = 0;
    do {
      char item_id[256];
      int count;
      if (2 != sscanf(arr, "%255[^*] * %d", item_id, &count)) {
        // TODO(micahdbak): error
        std::exit(1);
      }
      item_id[255] = '\0';

      GameHud::Item item{item_id, count};
      if (keep(item)) ret.push_back(item);

      while (*arr != '\0' && *arr != ',') arr++;

      if (*arr == ',') arr++;

      if (*arr == '\0') break;
    } while (i++ < 100);
  }

  return ret;
}

void Mouse::place(float x, float y) {
  this->x = x;
  this->y = y;
  this->update_path();
}

bool Mouse::check_collision(float x, float y) {
  return thoom::Scene::instance->map->collider->point_in_collider(x + 2.0f,
                                                                  y) ||
         thoom::Scene::instance->map->collider->point_in_collider(x - 2.0f,
                                                                  y) ||
         thoom::Scene::instance->map->collider->point_in_collider(x, y - 2.0f);
}

void Mouse::update_path() {
  int new_tile_x = (int)this->x / thoom::Scene::instance->map->tile_width;
  int new_tile_y = (int)this->y / thoom::Scene::instance->map->tile_height;
  if (new_tile_x == this->tile_x && new_tile_y == this->tile_y) return;

  this->tile_x = new_tile_x;
  this->tile_y = new_tile_y;
  foe_path_find(this);
}

void Mouse::draw() {
  this->dst_rect.x = float(int(this->x)) - 16.0f;
  this->dst_rect.y = float(int(this->y)) - 24.0f;
  thoom::Renderer::instance->push_sprite(
      this->is_feta ? "" : this->sprite->tex_id, this->sprite->texture,
      &this->sprite->frame, &this->dst_rect, 22);

  this->emote_rect.x = this->dst_rect.x;
  this->emote_rect.y = this->dst_rect.y - 11.0f;

  if (this->which_emote != -1) {
    this->emotes->set_frame(this->which_emote % 8);
    thoom::Renderer::instance->push_sprite(
        this->is_feta ? "" : this->emotes->tex_id, this->emotes->texture,
        &this->emotes->frame, &this->emote_rect, 34);
  }
}

std::string Mouse::selected_item() const {
  if (this->sel_item < 0 || this->sel_item >= (int)this->items.size()) {
    return ITEM_NONE;
  }
  return this->items[this->sel_item].item_id;
}

void Mouse::move(int x_dir, int y_dir, float mov_speed) {
  if (mov_speed <= 0.0f || (x_dir == 0 && y_dir == 0)) return;

  // new coordinates calculated with direction moving, movement speed, and
  // diagonal multiplier (if necessary)
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
  this->update_path();
}
