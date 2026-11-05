#include "frog.h"

#include "../items/cannon_ball.h"
#include "../items/cheese.h"
#include "audio.h"
#include "hurtbox.h"
#include "icon.h"
#include "renderer.h"
#include "save_data.h"
#include "scene.h"

FoeFrog::FoeFrog(int x, int y, int spawner_id)
    : Foe(float(x), float(y), spawner_id, 500, 250, 128.0f, 64.0f) {
  this->sprite = new thoom::Sprite("sprites/foe_frog.bmp", 48, 48, 125);
  this->dst_rect.w = 48.0f;
  this->dst_rect.h = 48.0f;
  this->icon_offset = 16;

  thoom::Scene::instance->create_object(
      HURTBOX_OBJ, HurtBox::options(this->id, -16, -16, 32, 32));
}

FoeFrog::~FoeFrog() {
  delete this->sprite;
  this->sprite = nullptr;
}

void FoeFrog::action(Mouse* mouse) {
  if (thoom::Scene::instance->ticks - this->attack_timer < 2000 ||
      thoom::Scene::instance->ticks - this->hurt_timer < 750) {
    this->state = Foe::State::FORCE_RANDOM_TILE;
    return;
  }

  this->attack_timer = thoom::Scene::instance->ticks;

  this->state = Foe::State::ACTION;
  this->timer = thoom::Scene::instance->ticks;

  thoom::Audio::instance->play("sfx/cannon.wav", 1.0, this->x, this->y, false);

  char options[256];
  snprintf(options, sizeof(options), "%d,%d,%d,%d,%d",
           (int)(this->x + this->off_x), (int)(this->y + this->off_y),
           this->x_dir, this->y_dir, this->id);
  thoom::Scene::instance->create_object(std::string(ITEM_CANNON_BALL USE_OBJ),
                                        std::string(options));

  // for action animation
  this->sprite->set_frame(0);
}

void FoeFrog::attack_internal(int damage) {
  this->health -= damage;

  if (this->health <= 0) {
    this->health = 0;
    this->state = Foe::State::DEAD;
    this->remove_from_foes();

    int kills = thoom::save.geti(FOE_FROG_OBJ STATS) + 1;
    thoom::save.puti(FOE_FROG_OBJ STATS, kills);
  } else {
    this->state = Foe::State::THROW_AWAY_FROM;
    this->hurt_timer = thoom::Scene::instance->ticks;
  }

  this->timer = thoom::Scene::instance->ticks;
}

#define WALK_ANIMATION 0
#define ACTION_ANIMATION 8

void FoeFrog::step() {
  this->foe_step();

  switch (this->state) {
    case Foe::State::IDLE:
      if (this->prev_state != this->state) {
        switch (SDL_rand(2)) {
          case 0:
            thoom::Audio::instance->play("sfx/tank_roll1.wav", 0.5, this->x,
                                         this->y, false);
            break;
          case 1:
            thoom::Audio::instance->play("sfx/tank_roll2.wav", 0.5, this->x,
                                         this->y, false);
            break;
        }
      }

      break;

    case Foe::State::ACTION:
      this->sprite->set_animation(ACTION_ANIMATION +
                                  this->_displayed_direction);
      if (thoom::Scene::instance->ticks - this->timer > 500) {
        this->state = Foe::State::FORCE_RANDOM_TILE;
      }

      break;

    case Foe::State::THROWN:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/tank_hurt.wav", 1.0, this->x, this->y,
                                     false);
      }

      break;

    case Foe::State::DEAD:
      if (this->prev_state != this->state) {
        thoom::Audio::instance->play("sfx/tank_die.wav", 1.0, this->x, this->y,
                                     false);
      }

      this->sprite->set_animation(ACTION_ANIMATION +
                                  this->_displayed_direction);
      this->sprite->set_frame(0);
      if (thoom::Scene::instance->ticks - this->timer > 2000) {
        // delete this object
        this->deleted = true;
        Cheese::drop_cheese(this->x, this->y, 3, 5);

        char options[256];
        snprintf(options, sizeof(options), "%d,%d", int(this->x + this->off_x),
                 int(this->y + this->off_y));
        thoom::Scene::instance->create_object(
            std::string(ITEM_CANNON_BALL DROPPED_OBJ), std::string(options));

        return;
      }

      push_icon(SKULL_AND_BONES_ICON, this->x + this->off_x,
                this->y - 24.0f + this->off_y, &this->icon_src,
                &this->icon_dst);

      break;

    default:
      this->sprite->set_animation(WALK_ANIMATION + this->_displayed_direction);

      break;
  }

  if (thoom::Scene::instance->ticks - this->hurt_timer < 500 &&
      this->state != Foe::State::DEAD) {
    this->sprite->set_animation(ACTION_ANIMATION + this->_displayed_direction);
    this->sprite->set_frame(0);
    push_health_bar(this->health, this->max_health, this->x + this->off_x,
                    this->y - 24.0f + this->off_y, &this->icon_src,
                    &this->icon_dst);
  }

  this->prev_state = this->state;

  this->sprite->update_frame();
  this->dst_rect.x = this->x - 24.0f + this->off_x;
  this->dst_rect.y = this->y - 24.0f + this->off_y;
  thoom::Renderer::instance->push_sprite(
      this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
      &this->dst_rect, 32);
}

thoom::Object* FoeFrog::create_object(thoom::Scene* scene,
                                      const std::string& options) {
  int x = 0, y = 0, spawner_id = 0;
  if (3 != sscanf(options.c_str(), "%d,%d,%d", &x, &y, &spawner_id)) {
    // TODO(micahdbak): error
    std::exit(1);
  }

  return new FoeFrog(x, y, spawner_id);
}
