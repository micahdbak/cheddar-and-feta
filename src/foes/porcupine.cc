#include "porcupine.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "audio/audio.h"
#include "foes/foe.h"
#include "hurtbox.h"
#include "icon.h"
#include "items/cheese.h"
#include "items/item.h"
#include "items/toothpick.h"
#include "mouse.h"
#include "object.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "sprite/sprite.h"
#include "utils/utils.h"

FoePorcupine::FoePorcupine(int x, int y, int spawner_id)
    : Foe(float(x), float(y), spawner_id, 250, 100, 256.0f, 64.0f) {
  this->sprite = new thoom::Sprite("sprites/foe_porcupine.bmp", 32, 32, 250);

  this->dst_rect.w = 32.0f;
  this->dst_rect.h = 32.0f;

  thoom::Scene::instance->create_object(
      HURTBOX_OBJ, HurtBox::options(this->id, -6, -8, 12, 16));
}

FoePorcupine::~FoePorcupine() {
  delete this->sprite;
  this->sprite = nullptr;
}

#define WALK_ANIMATION 0
#define ATTACK_ANIMATION 8
#define DEAD_ANIMATION 16

void FoePorcupine::action(Mouse* mouse) {
  if (thoom::Scene::instance->ticks - this->hurt_timer < 500) {
    this->state = Foe::State::FORCE_RANDOM_TILE;
    return;
  }

  thoom::dir_to_point(this->x, this->y, mouse->x, mouse->y, &this->spine_x_dir,
                      &this->spine_y_dir);
  this->sprite->set_animation(
      ATTACK_ANIMATION +
      thoom::direction_from_dirs(this->spine_x_dir, this->spine_y_dir));
  this->sprite->set_frame(0);
  this->timer = thoom::Scene::instance->ticks;
}

void FoePorcupine::attack_internal(int damage) {
  this->health -= damage;

  if (this->health <= 0) {
    this->health = 0;
    this->state = Foe::State::DEAD;
    this->remove_from_foes();

    int kills = thoom::save.geti(FOE_PORCUPINE_OBJ STATS) + 1;
    thoom::save.puti(FOE_PORCUPINE_OBJ STATS, kills);
  } else {
    this->state = Foe::State::THROW_AWAY_FROM;
    this->hurt_timer = thoom::Scene::instance->ticks;
  }

  this->timer = thoom::Scene::instance->ticks;
}

void FoePorcupine::step() {
  this->foe_step();

  switch (this->state) {
    case Foe::State::ACTION:
      this->sprite->interval_ms = 200;
      if (thoom::Scene::instance->ticks - this->timer > 600) {
        this->state = Foe::State::IDLE;
        this->tile_choice = Foe::TileChoice::AWAY;
      } else if (thoom::Scene::instance->ticks - this->timer > 400 &&
                 (this->spine_x_dir != 0 || this->spine_y_dir != 0)) {
        char buff[256];
        snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d",
                 (int)(this->x + this->off_x), (int)(this->y + this->off_y),
                 this->spine_x_dir, this->spine_y_dir, this->id);
        thoom::Scene::instance->create_object(ITEM_TOOTHPICK USE_OBJ,
                                              std::string(buff));
        this->spine_x_dir = this->spine_y_dir = 0;
        thoom::Audio::instance->play("sfx/throw.wav", 1.0f, this->x, this->y,
                                     false);
      }

      break;

    case Foe::State::THROWN:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/ant_hurt.wav", 1.0f, this->x, this->y,
                                     false);
      }

      break;

    case Foe::State::DEAD:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/ant_die.wav", 1.0f, this->x, this->y,
                                     false);
      }

      this->sprite->set_animation(DEAD_ANIMATION + this->direction);
      if (thoom::Scene::instance->ticks - this->timer > 2000) {
        // delete this object
        this->deleted = true;
        Cheese::drop_cheese(this->x, this->y, 3, 5);

        return;
      }

      push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x,
                this->y - 16.0f + this->off_y, &this->icon_src,
                &this->icon_dst);

      break;

    default:
      this->sprite->set_animation(WALK_ANIMATION + this->direction);
      this->sprite->interval_ms = 100;

      if (this->tile_choice == Foe::TileChoice::AWAY &&
          this->current_distance >= 48.0f) {
        this->tile_choice = Foe::TileChoice::CIRCLE;
      }

      break;
  }

  this->prev_state = this->state;

  if (thoom::Scene::instance->ticks - this->hurt_timer < 250 &&
      this->state != Foe::State::DEAD) {
    this->sprite->set_animation(WALK_ANIMATION);
    this->sprite->set_frame(1);
    push_health_bar(this->health, this->max_health, this->x + this->off_x,
                    this->y - 12.0f + this->off_y, &this->icon_src,
                    &this->icon_dst);
  }

  if (this->sprite->update_frame() && this->state == Foe::State::WALKING &&
      (this->sprite->frame_i % 2) == 1) {
    thoom::Audio::instance->play("sfx/step.wav", 1.0f, this->x, this->y, false);
  }

  this->dst_rect.x = this->x - 16.0f + this->off_x;
  this->dst_rect.y = this->y - 16.0f + this->off_y;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 22);
}

thoom::Object* FoePorcupine::create_object(thoom::Scene* scene,
                                           const std::string& options) {
  int x = 0, y = 0, spawner_id = 0;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new FoePorcupine(x, y, spawner_id);
}
