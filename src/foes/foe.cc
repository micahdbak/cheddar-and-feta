#include "foe.h"

#include <vector>

#include "hitbox.h"
#include "mouse.h"
#include "object/entity.h"
#include "scene/scene.h"
#include "spawner.h"
#include "thoom/macros.h"
#include "utils/utils.h"

static std::vector<int> speed_offsets = {0, 4, 8, 4, 2, -2, -4, -8, -4};
static int speed_offsets_i = 0;

Foe::Foe(const thoom::Entity::Context& context, float x, float y,
         int spawner_id, int speed, int throw_speed, float stalking_distance,
         float action_distance)
    : thoom::Entity(context),
      spawner_id(spawner_id),
      speed(speed),
      throw_speed(throw_speed),
      stalking_distance(stalking_distance),
      action_distance(action_distance) {
  this->target_x = (int)x / thoom::Scene::instance->map->tile_width;
  this->target_y = (int)y / thoom::Scene::instance->map->tile_height;

  // ensure starting x/y is in the center of the corresponding tile
  this->x = center_tile_x(this->target_x);
  this->y = center_tile_y(this->target_y);

  this->state = Foe::State::IDLE;

  this->speed += speed_offsets[speed_offsets_i];
  speed_offsets_i++;

  if (speed_offsets_i >= speed_offsets.size()) speed_offsets_i = 0;

  switch (speed_offsets_i % 3) {
    case 1:
      this->strafe = FoeStrafe::STRAFE_LEFT;
      break;
    case 2:
      this->strafe = FoeStrafe::STRAFE_RIGHT;
      break;
    default:
      this->strafe = FoeStrafe::NO_STRAFE;
      break;
  }

  this->off_x = SDL_randf() * 16.0f - 8.0f;
  this->off_y = SDL_randf() * 16.0f - 8.0f;
}

Foe::~Foe() {
  if (this->spawner_id >= 0 && spawners[this->spawner_id] != nullptr) {
    spawners[this->spawner_id]->log_death();
  }
}

void Foe::attack(int damage) {
  if (this->state == Foe::State::HURT || this->state == Foe::State::THROWN ||
      this->state == Foe::State::DEAD) {
    return;
  }

  this->attack_internal(damage);
  this->walk_offset = thoom::Scene::instance->ticks - this->start_ticks;
}

void Foe::receive_hit(const Hit& hit) {
  this->attack(hit.damage);

  if (hit.throw_away) {
    thoom::dir_to_point(hit.contact_x, hit.contact_y, this->x, this->y,
                        &this->throw_x, &this->throw_y);
  } else {
    thoom::dir_to_point(this->x, this->y, hit.contact_x, hit.contact_y,
                        &this->throw_x, &this->throw_y);
  }
}

bool Foe::hittable() const { return this->state != Foe::State::DEAD; }

void Foe::foe_step() {
  switch (this->state) {
    case Foe::State::IDLE: {
      Mouse* new_mouse = Mouse::closest_mouse(this->x, this->y,
                                              this->stalking_distance, false);

      if (new_mouse == nullptr) {
        // no more stalking if neither mouse is close enough
        this->target_mouse = nullptr;
        this->current_distance = this->stalking_distance;
      } else if (this->target_mouse == nullptr) {
        // no target mouse presently; target the new mouse
        Uint64 target_ticks_offset = SDL_rand(10) * 1000;
        this->target_ticks =
            thoom::Scene::instance->ticks - target_ticks_offset;
        this->target_mouse = new_mouse;
        this->current_distance = thoom::distance_between_points(
            this->x, this->y, new_mouse->x, new_mouse->y);
      } else {
        this->current_distance = thoom::distance_between_points(
            this->x, this->y, this->target_mouse->x, this->target_mouse->y);

        if (new_mouse != this->target_mouse &&
            (this->target_mouse->is_down ||
             this->current_distance > this->stalking_distance ||
             thoom::Scene::instance->ticks - this->target_ticks > 5000)) {
          this->target_ticks = thoom::Scene::instance->ticks;
          this->target_mouse = new_mouse;
          this->current_distance = thoom::distance_between_points(
              this->x, this->y, new_mouse->x, new_mouse->y);
        }
      }

      // perform action and stay on this tile if close enough and not running
      // away
      if (this->tile_choice != TileChoice::SPAZZ &&
          this->current_distance < this->action_distance) {
        this->action(this->target_mouse);
        this->state = Foe::State::ACTION;
        break;
      }

      // will be walking if not action'ing
      this->state = Foe::State::WALKING;

      int current_x = (int)this->x / thoom::Scene::instance->map->tile_width;
      int current_y = (int)this->y / thoom::Scene::instance->map->tile_height;
      if (current_x >= 0 && current_y >= 0 &&
          current_x < thoom::Scene::instance->map->cols &&
          current_y < thoom::Scene::instance->map->rows &&
          thoom::Scene::instance->map->collider->get(current_x, current_y) <
              0) {
        this->target_x = current_x;
        this->target_y = current_y;
      } else {
        current_x = this->target_x;
        current_y = this->target_y;
      }

      // move towards closest mouse
      if (this->target_mouse != nullptr) {
        bool path_exists = false;
        switch (this->tile_choice) {
          case Foe::TileChoice::TOWARDS:
            path_exists = foe_move_towards(this->target_mouse, &this->target_x,
                                           &this->target_y, this->strafe);
            break;
          case Foe::TileChoice::AWAY:
            path_exists = foe_move_away(this->target_mouse, &this->target_x,
                                        &this->target_y);
            break;
          case Foe::TileChoice::CIRCLE:
            path_exists = foe_move_circle(this->target_mouse, &this->target_x,
                                          &this->target_y);
            break;
          case Foe::TileChoice::SPAZZ:
            if (SDL_rand(2)) {
              path_exists = foe_move_circle(this->target_mouse, &this->target_x,
                                            &this->target_y);
            } else {
              path_exists = false;  // random tile
            }
            break;
        }

        // we are on top of the mouse - pick a random tile
        if (!path_exists) {
          foe_pick_random(&this->target_x, &this->target_y);
        }
      } else {
        // no mouse close enough; pick a random tile and move there
        foe_pick_random(&this->target_x, &this->target_y);
      }

      this->x_dir = this->target_x - current_x;
      this->y_dir = this->target_y - current_y;
      this->direction = thoom::direction_from_dirs(x_dir, y_dir);

      if (this->x_dir == 0 && this->y_dir == 0) {
        this->state = Foe::State::IDLE;
      }

      this->start_x = this->x;
      this->start_y = this->y;
      this->start_ticks = thoom::Scene::instance->ticks;

      bool diagonal = this->x_dir != 0 && this->y_dir != 0;
      this->walking_time =
          diagonal ? (int)((float)this->speed * THOOM_SQRT2_F) : this->speed;

      float target_center_x = center_tile_x(this->target_x);
      float target_center_y = center_tile_x(this->target_y);
      this->travel_w = target_center_x - this->start_x;
      this->travel_h = target_center_y - this->start_y;
    } break;

    case Foe::State::FORCE_RANDOM_TILE: {
      this->state = Foe::State::WALKING;

      int current_x = (int)this->x / thoom::Scene::instance->map->tile_width;
      int current_y = (int)this->y / thoom::Scene::instance->map->tile_height;
      if (current_x >= 0 && current_y >= 0 &&
          current_x < thoom::Scene::instance->map->cols &&
          current_y < thoom::Scene::instance->map->rows &&
          thoom::Scene::instance->map->collider->get(current_x, current_y) <
              0) {
        this->target_x = current_x;
        this->target_y = current_y;
      } else {
        current_x = this->target_x;
        current_y = this->target_y;
      }

      foe_pick_random(&this->target_x, &this->target_y);

      this->x_dir = this->target_x - current_x;
      this->y_dir = this->target_y - current_y;
      this->direction = thoom::direction_from_dirs(x_dir, y_dir);

      if (this->x_dir == 0 && this->y_dir == 0) {
        this->state = Foe::State::IDLE;
      }

      this->start_x = this->x;
      this->start_y = this->y;
      this->start_ticks = thoom::Scene::instance->ticks;

      bool diagonal = this->x_dir != 0 && this->y_dir != 0;
      this->walking_time =
          diagonal ? (int)((float)this->speed * THOOM_SQRT2_F) : this->speed;

      float target_center_x = center_tile_x(this->target_x);
      float target_center_y = center_tile_x(this->target_y);
      this->travel_w = target_center_x - this->start_x;
      this->travel_h = target_center_y - this->start_y;
    } break;

    case Foe::State::THROW_AWAY_FROM: {
      this->state = Foe::State::THROWN;

      int current_x = (int)this->x / thoom::Scene::instance->map->tile_width;
      int current_y = (int)this->y / thoom::Scene::instance->map->tile_height;
      if (current_x >= 0 && current_y >= 0 &&
          current_x < thoom::Scene::instance->map->cols &&
          current_y < thoom::Scene::instance->map->rows &&
          thoom::Scene::instance->map->collider->get(current_x, current_y) <
              0) {
        this->target_x = current_x;
        this->target_y = current_y;
      } else {
        current_x = this->target_x;
        current_y = this->target_y;
      }

      foe_move_direction(&this->target_x, &this->target_y, this->throw_x,
                         this->throw_y);

      this->x_dir = this->target_x - current_x;
      this->y_dir = this->target_y - current_y;
      this->direction = thoom::direction_from_dirs(x_dir, y_dir);

      if (this->x_dir == 0 && this->y_dir == 0) {
        this->state = Foe::State::IDLE;
      }

      this->start_x = this->x;
      this->start_y = this->y;
      this->start_ticks = thoom::Scene::instance->ticks;

      bool diagonal = this->x_dir != 0 && this->y_dir != 0;
      this->walking_time = diagonal
                               ? (int)((float)this->throw_speed * THOOM_SQRT2_F)
                               : this->throw_speed;

      float target_center_x = center_tile_x(this->target_x);
      float target_center_y = center_tile_x(this->target_y);
      this->travel_w = target_center_x - this->x;
      this->travel_h = target_center_y - this->y;
    }; break;

    case Foe::State::WALKING:
    case Foe::State::THROWN: {
      int elapsed = thoom::Scene::instance->ticks - this->start_ticks;
      float perc = (float)elapsed / (float)this->walking_time;

      this->x = this->start_x + (perc * this->travel_w);
      this->y = this->start_y + (perc * this->travel_h);

      if (elapsed > this->walking_time) {
        this->x = center_tile_x(this->target_x);
        this->y = center_tile_y(this->target_y);
        this->state = Foe::State::IDLE;  // next step will select a new tile
      }

      // exclusive to WALKING: update this->_displayed_direction
      if (this->state == Foe::State::WALKING &&
          this->_displayed_direction != this->direction &&
          thoom::Scene::instance->ticks - this->direction_timer > 100) {
        this->direction_timer = thoom::Scene::instance->ticks;

        int diff_up =
            (this->direction < this->_displayed_direction ? this->direction + 8
                                                          : this->direction) -
            this->_displayed_direction;
        int diff_down =
            this->_displayed_direction -
            (this->direction > this->_displayed_direction ? this->direction - 8
                                                          : this->direction);

        if (diff_up <= diff_down) {
          this->_displayed_direction++;
        } else {
          // diff_down < diff_up
          this->_displayed_direction--;
        }

        if (this->_displayed_direction < 0)
          this->_displayed_direction = 7;
        else if (this->_displayed_direction > 7)
          this->_displayed_direction = 0;
      }
    } break;

    case HURT: {
      // for when out of the hurt
      this->start_ticks = thoom::Scene::instance->ticks - this->walk_offset;
    } break;

    default:
      break;
  }
}
