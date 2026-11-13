#include "bug.h"

#include <cstdio>
#include <cstdlib>

#include "audio/audio.h"
#include "foes/foe.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "icon.h"
#include "items/cheese.h"
#include "mouse.h"
#include "object/entity.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"

FoeBug::FoeBug(const thoom::Entity::Context& context, int x, int y,
               int spawner_id)
    : Foe(context, float(x), float(y), spawner_id,
          spawner_id == -2 ? 1000 : 500, 100, 256.0f, 16.0f) {
  this->sprite = new thoom::Sprite("sprites/foe_bug.bmp", 32, 32, 100);

  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;

  new HurtBox(*this, -8, -8, 16, 16);
}

FoeBug::~FoeBug() {
  delete this->sprite;
  this->sprite = nullptr;
}

void FoeBug::action(Mouse* mouse) {
  if (thoom::Scene::instance->ticks - this->hurt_timer < 500) {
    this->state = Foe::State::FORCE_RANDOM_TILE;
    return;
  }

  this->state = Foe::State::ACTION;
  this->timer = thoom::Scene::instance->ticks;

  int x_off, y_off;
  HitBox::make_offset(x_dir, y_dir, &x_off, &y_off, 8.0f);
  HitBox::Properties props = {1, 500, 250};
  new HitBox(*this, this, x_off - 8, y_off - 8, 16, 16, props);
}

void FoeBug::attack_internal(int damage) {
  this->health -= damage;

  if (this->health <= 0) {
    this->health = 0;
    this->state = Foe::State::DEAD;
    this->remove_from_foes();

    int kills = thoom::save.geti(FOE_BUG_OBJ STATS) + 1;
    thoom::save.puti(FOE_BUG_OBJ STATS, kills);
  } else {
    this->state = Foe::State::THROW_AWAY_FROM;
    this->hurt_timer = thoom::Scene::instance->ticks;
  }

  this->timer = thoom::Scene::instance->ticks;
}

#define WALK_ANIMATION 0
#define ATTACK_ANIMATION 0
#define HURT_ANIMATION 8

void FoeBug::step() {
  this->foe_step();

  switch (this->state) {
    case Foe::State::IDLE:
      if (this->prev_state != this->state) {
        switch (SDL_rand(4)) {
          case 0:
            thoom::Audio::instance->play("sfx/ant_walk1.wav", 0.5, this->x,
                                         this->y, false);
            break;
          case 1:
            thoom::Audio::instance->play("sfx/ant_walk2.wav", 0.5, this->x,
                                         this->y, false);
            break;
          case 2:
            thoom::Audio::instance->play("sfx/ant_walk3.wav", 0.5, this->x,
                                         this->y, false);
            break;
          default:
            break;  // no sound
        }
      }

      break;

    case Foe::State::ACTION:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/ant_attack.wav", 1.0, this->x,
                                     this->y, false);
      }

      this->sprite->set_animation(ATTACK_ANIMATION +
                                  this->_displayed_direction);
      this->sprite->interval_ms = 50;
      if (thoom::Scene::instance->ticks - this->timer > 1000) {
        this->state = Foe::State::FORCE_RANDOM_TILE;
      }

      break;

    case Foe::State::THROWN:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/ant_hurt.wav", 1.0, this->x, this->y,
                                     false);
      }

      // foe internally exits the thrown state

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
        Cheese::drop_cheese(this->x, this->y, 1, 1);

        return;
      }

      break;

    default:
      this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);
      this->sprite->interval_ms = 100;

      break;
  }

  if (thoom::Scene::instance->ticks - this->hurt_timer < 333 &&
      this->state != Foe::State::DEAD) {
    this->sprite->set_animation(HURT_ANIMATION + this->_displayed_direction);
  }

  this->prev_state = this->state;

  this->sprite->update_frame();
}

void FoeBug::render() {
  if (this->state == Foe::State::DEAD) {
    push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x,
              this->y - 14.0f + this->off_y, &this->icon_src, &this->icon_dst);
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
      &this->dst_rect, 18);
}

thoom::Entity* FoeBug::create_entity(const thoom::Entity::Context& context,
                                     const std::string& options) {
  int x = 0, y = 0, spawner_id = 0;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new FoeBug(context, x, y, spawner_id);
}
