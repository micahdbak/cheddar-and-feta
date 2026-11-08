#include "spitter.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "abdomen.h"
#include "audio/audio.h"
#include "foes/foe.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "icon.h"
#include "items/fire.h"
#include "items/item.h"
#include "mouse.h"
#include "object.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "segment.h"
#include "sprite/sprite.h"
#include "thoom/macros.h"
#include "utils/utils.h"

Spitter::Spitter(float x, float y, int spawner_id)
    : Foe(x, y, spawner_id, 200, 250, 256.0f, 80.0f) {
  // make NUM_SEGMENTS-1 segments (the NUM_SEGMENTS'th segment is the abdomen)
  for (int i = 1; i < NUM_SEGMENTS; i++) {
    thoom::Scene::instance->create_object(
        FOE_SPITTER_SEGMENT_OBJ,
        std::to_string(this->gid) + "," + std::to_string(i));
  }

  thoom::Scene::instance->create_object(FOE_SPITTER_ABDOMEN_OBJ,
                                        std::to_string(this->gid));

  this->off_x = 0.0f;
  this->off_y = 0.0f;

  this->push_step(this->x, this->y, 0);

  this->sprite = new thoom::Sprite("sprites/foe_spitter_head.bmp", 32, 32, 0);
  this->dst_rect.w = this->dst_rect.h = 32.0f;

  this->start_timer = thoom::Scene::instance->ticks;
}

Spitter::~Spitter() { delete this->sprite; }

void Spitter::action(Mouse* mouse) {
  this->timer = thoom::Scene::instance->ticks;
}

void Spitter::attack_internal(int damage) {
  if (hurtbox_hitbox_gid == HB_SHARED_FIRE ||
      hurtbox_hitbox_gid == HB_SHARED_SPITTER) {
    return;  // fire does no damage
  }

  this->health -= damage;

  if (this->health <= 0) {
    this->health = 0;
    this->state = Foe::State::DEAD;
    this->remove_from_foes();

    thoom::save.puti(FOE_SPITTER_OBJ STATS,
                     thoom::Scene::instance->ticks - this->start_timer);
  } else {
    this->state = Foe::State::HURT;
  }

  this->hurt_timer = thoom::Scene::instance->ticks;
}

void Spitter::step() {
  this->foe_step();

  switch (this->state) {
    case Foe::State::IDLE:
      if (this->prev_state != this->state) {
        switch (SDL_rand(6)) {
          case 0:
            thoom::Audio::instance->play("sfx/spitter_walk1.wav", 1.0f, this->x,
                                         this->y, false);
            break;
          case 1:
            thoom::Audio::instance->play("sfx/spitter_walk2.wav", 1.0f, this->x,
                                         this->y, false);
            break;
          case 2:
            thoom::Audio::instance->play("sfx/spitter_walk3.wav", 1.0f, this->x,
                                         this->y, false);
            break;
          case 3:
            thoom::Audio::instance->play("sfx/spitter_walk4.wav", 1.0f, this->x,
                                         this->y, false);
            break;
          case 4:
            thoom::Audio::instance->play("sfx/ant_walk3.wav", 1.0f, this->x,
                                         this->y, false);
            break;
          default:
            break;
        }
      }

      break;

    case Foe::State::ACTION: {
      Uint64 action_offset = thoom::Scene::instance->ticks - this->timer;
      if (action_offset > 2000) {
        this->tile_choice = Foe::TileChoice::SPAZZ;
        this->state = Foe::State::WALKING;
        break;
      }

      Mouse* mouse = Mouse::closest_mouse(this->x, this->y, 80.0f, true);

      if (mouse == nullptr) {
        this->state = Foe::State::IDLE;
        break;
      } else {
        float distance = thoom::distance_between_points(this->x, this->y,
                                                        mouse->x, mouse->y);
        if (distance < 32.0f || this->abdomen_distance < 32.0f) {
          this->tile_choice = Foe::TileChoice::SPAZZ;
          this->state = Foe::State::IDLE;
          break;
        }
      }

      int action_direction = 0;

      int x_dir, y_dir;
      thoom::dir_to_point(this->x, this->y, mouse->x, mouse->y, &x_dir, &y_dir);
      action_direction = thoom::direction_from_dirs(x_dir, y_dir);

      int direction_offset = 0;

      switch (action_offset / 250) {
        case 0:
          direction_offset = 0;
          break;
        case 1:
          direction_offset = -1;
          break;
        case 2:
          direction_offset = 0;
          break;
        case 3:
          direction_offset = 1;
          break;
        case 4:
          direction_offset = 0;
          break;
        case 5:
          direction_offset = -1;
          break;
        case 6:
          direction_offset = 0;
          break;
        case 7:
          direction_offset = 1;
          break;
        default:
          break;
      }

      this->direction = action_direction + direction_offset;
      if (this->direction < 0)
        this->direction = 7;
      else if (this->direction > 7)
        this->direction = 0;

      if (last_direction != this->direction &&
          this->direction == this->displayed_direction) {
        last_direction = this->direction;

        int distance =
            (int)(x_dir != 0 && y_dir != 0 ? 16.0f * THOOM_INV_SQRT2_F : 16.0f);
        float fire_x = this->x + (float)(x_dir * distance);
        float fire_y = this->y + (float)(y_dir * distance);

        int fire_x_dir, fire_y_dir;
        thoom::dirs_from_direction(this->direction, &fire_x_dir, &fire_y_dir);
        thoom::Scene::instance->create_object(
            ITEM_FIRE USE_OBJ, UseItem::options(fire_x, fire_y, fire_x_dir * 2,
                                                fire_y_dir * 2, -1));

        if (SDL_rand(2)) {
          thoom::Audio::instance->play("sfx/fire_short1.wav", 1.0f, this->x,
                                       this->y, false);
        } else {
          thoom::Audio::instance->play("sfx/fire_short2.wav", 1.0f, this->x,
                                       this->y, false);
        }
      }
    } break;

    case Foe::State::HURT:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/spitter_hurt.wav", 1.0f, this->x,
                                     this->y, false);
      }

      if (thoom::Scene::instance->ticks - this->hurt_timer > 100) {
        this->state = Foe::State::WALKING;
      }

      break;

    case Foe::State::DEAD:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/spitter_die.wav", 1.0f, this->x,
                                     this->y, false);
      }

      if (thoom::Scene::instance->ticks - this->hurt_timer > 2000) {
        // delete this object
        this->deleted = true;
        return;
      }

      break;

    default:
      if (this->tile_choice == Foe::TileChoice::SPAZZ &&
          this->abdomen_distance > 64.0f) {
        this->tile_choice = Foe::TileChoice::TOWARDS;
      }

      break;
  }

  this->prev_state = this->state;

  if (thoom::Scene::instance->ticks - this->direction_timer > 100 &&
      this->displayed_direction != this->direction) {
    this->direction_timer = thoom::Scene::instance->ticks;

    int diff_up =
        (this->direction < this->displayed_direction ? this->direction + 8
                                                     : this->direction) -
        this->displayed_direction;
    int diff_down =
        this->displayed_direction - (this->direction > this->displayed_direction
                                         ? this->direction - 8
                                         : this->direction);

    if (diff_up <= diff_down) {
      this->displayed_direction++;
    } else {
      // diff_down < diff_up
      this->displayed_direction--;
    }

    if (this->displayed_direction < 0)
      this->displayed_direction = 7;
    else if (this->displayed_direction > 7)
      this->displayed_direction = 0;
  }

  // get last recorded position
  struct Spitter::step_t pos = this->get_pos(0);

  if (pos.x - (int)this->x != 0 || pos.y - (int)this->y != 0) {
    this->push_step(this->x, this->y, this->displayed_direction);
  }

  this->sprite->set_animation(this->displayed_direction);
}

void Spitter::render() {
  if (this->state == Foe::State::DEAD) {
    push_icon(SKULL_AND_BONES_ICON, this->x, this->y - 14.0f, &this->icon_src,
              &this->icon_dst);
  }

  if (thoom::Scene::instance->ticks - this->hurt_timer < 500 &&
      this->state != Foe::State::DEAD) {
    push_health_bar(this->health, this->max_health, this->x, this->y - 16.0f,
                    &this->icon_src, &this->icon_dst);
  }

  this->dst_rect.x = this->x - 16.0f;
  this->dst_rect.y = this->y - 16.0f;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 16);
}

thoom::Object* Spitter::create_object(thoom::Scene* scene,
                                      const std::string& options) {
  int x, y, spawner_id;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new Spitter(x, y, spawner_id);
}
