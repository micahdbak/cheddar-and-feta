#include "bat.h"

#include <math.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "audio/audio.h"
#include "foes/foe.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "icon.h"
#include "items/cheese.h"
#include "mouse.h"
#include "object.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

#define CIRCLE_TICKS 2000

FoeBat::FoeBat(int x, int y, int spawner_id)
    : Foe(float(x), float(y), spawner_id, 175, 100, 256.0f, 16.0f) {
  this->sprite = new thoom::Sprite("sprites/foe_bat.bmp", 32, 32,
                                   45);  // approx every 3 frames

  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;

  thoom::Scene::instance->create_object(
      HURTBOX_OBJ, HurtBox::options(this->gid, -8, -8, 16, 16));

  this->base_ticks_offset = SDL_rand(CIRCLE_TICKS + 1);
  this->ticks_offset = base_ticks_offset;
}

FoeBat::~FoeBat() {
  delete this->sprite;
  this->sprite = nullptr;
}

void FoeBat::action(Mouse* mouse) {
  if (thoom::Scene::instance->ticks - this->timer < 500) {
    this->state = Foe::State::FORCE_RANDOM_TILE;
    return;
  }

  this->timer = thoom::Scene::instance->ticks;

  int x_off, y_off;
  HitBox::make_offset(x_dir, y_dir, &x_off, &y_off, 8.0f);
  HitBox::Properties props = {1, 500, 250};
  thoom::Scene::instance->create_object(
      HITBOX_OBJ, HitBox::options(this->gid, this->gid, x_off - 8, y_off - 8,
                                  16, 16, props));
}

void FoeBat::attack_internal(int damage) {
  this->health -= damage;

  if (this->health <= 0) {
    this->health = 0;
    this->state = Foe::State::DEAD;
    this->remove_from_foes();

    int kills = thoom::save.geti(FOE_BAT_OBJ STATS) + 1;
    thoom::save.puti(FOE_BAT_OBJ STATS, kills);
  } else {
    this->state = Foe::State::THROW_AWAY_FROM;
    this->hurt_timer = thoom::Scene::instance->ticks;
  }

  this->timer = thoom::Scene::instance->ticks;
}

#define WALK_ANIMATION 0
#define HURT_ANIMATION 8  // 16

void FoeBat::step() {
  this->foe_step();

  switch (this->state) {
    case Foe::State::IDLE:
      if (this->prev_state != this->state &&
          thoom::Scene::instance->ticks - this->audio_timer > 500) {
        this->audio_timer = thoom::Scene::instance->ticks;
        thoom::Audio::instance->play("sfx/ant_fly.wav", 0.5, this->x, this->y,
                                     false);
      }

      break;

    case Foe::State::ACTION:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/ant_attack.wav", 1.0, this->x,
                                     this->y, false);
      }

      if (thoom::Scene::instance->ticks - this->timer > 250) {
        this->state = State::FORCE_RANDOM_TILE;
      }

      break;

    case Foe::State::THROWN:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/ant_hurt.wav", 1.0, this->x, this->y,
                                     false);
      }

      break;

    case Foe::State::DEAD:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/ant_die.wav", 1.0, this->x, this->y,
                                     false);
      }

      this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);

      if (thoom::Scene::instance->ticks - this->timer > 2000) {
        // delete this object
        this->deleted = true;
        Cheese::drop_cheese(this->x, this->y, 1, 3);

        return;
      }

      break;

    default:
      this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);

      if (this->tile_choice == Foe::TileChoice::TOWARDS &&
          this->current_distance <= 32.0f) {
        this->tile_choice = Foe::TileChoice::CIRCLE;
      } else if (this->tile_choice == Foe::TileChoice::CIRCLE &&
                 this->current_distance >= 128.0f) {
        this->tile_choice = Foe::TileChoice::TOWARDS;
      }

      break;
  }

  if (thoom::Scene::instance->ticks - this->hurt_timer < 333 &&
      this->state != Foe::State::DEAD) {
    this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);
  }

  if (this->state != Foe::State::HURT && this->state != Foe::State::DEAD) {
    float angle = (float)((thoom::Scene::instance->ticks - this->ticks_offset) %
                          CIRCLE_TICKS) /
                  1000.0f;
    angle *= M_PI;
    this->off_x = sinf(angle) * 8.0f;
    this->off_y = cosf(angle) * 8.0f;
  } else {
    this->ticks_offset =
        thoom::Scene::instance->ticks - this->timer + base_ticks_offset;
  }

  this->prev_state = this->state;

  this->sprite->update_frame();
}

void FoeBat::render() {
  if (this->state == Foe::State::DEAD) {
    push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x,
              this->y - 16.0f + this->off_y, &this->icon_src, &this->icon_dst);
  }

  if (thoom::Scene::instance->ticks - this->hurt_timer < 333 &&
      this->state != Foe::State::DEAD) {
    push_health_bar(this->health, this->max_health, this->x + this->off_x,
                    this->y - 16.0f + this->off_y, &this->icon_src,
                    &this->icon_dst);
  }

  this->dst_rect.x = this->x - 16.0f + this->off_x;
  this->dst_rect.y = this->y - 16.0f + this->off_y;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 26);
}

thoom::Object* FoeBat::create_object(thoom::Scene* scene,
                                     const std::string& options) {
  int x = 0, y = 0, spawner_id = 0;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new FoeBat(x, y, spawner_id);
}
