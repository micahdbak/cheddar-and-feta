#include "splash.h"

#include <math.h>

#include <cmath>
#include <cstddef>
#include <cstdint>

#include "colour.h"
#include "controller.h"
#include "input.h"
#include "macros.h"
#include "object.h"
#include "renderer.h"
#include "scene.h"
#include "sprite.h"
#include "texture.h"
#include "window.h"

Splash::Splash() {
  this->overlay = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_LINEAR);
  this->tunnel = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_LINEAR);

  this->overlay_rect.x = this->overlay_rect.y = 0.0f;
  this->overlay_rect.w = THOOM_SCREEN_WIDTH;
  this->overlay_rect.h = THOOM_SCREEN_HEIGHT;

  this->animation = 0;

  this->sprite = new thoom::Sprite("sprites/splash.bmp", 128, 96, 250);
  this->sprite->set_animation(this->animation);
  this->dst_rect.w = 128.0f;
  this->dst_rect.h = 96.0f;

  this->ched_src = {16.0f, 400.0f, 32.0f, 32.0f};
  this->ched_dst.w = this->ched_dst.h = 32.0f;
  this->feta_src = {16.0f, 432.0f, 32.0f, 32.0f};
  this->feta_dst.w = this->feta_dst.h = 32.0f;

  this->ant = new thoom::Sprite("sprites/splash_ant.bmp", 32, 32, 100);

  this->initial_timer = thoom::Scene::instance->ticks;

  this->animations = 7;

  thoom::Renderer::instance->reset_view();
}

Splash::~Splash() {
  delete this->sprite;
  delete this->ant;
  SDL_DestroyTexture(this->overlay);
  SDL_DestroyTexture(this->tunnel);
}

#define DISPLAY_FOR 4500

void Splash::step() {
  this->timer = thoom::Scene::instance->ticks - this->initial_timer;

  // skip splash
  if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
    this->timer = DISPLAY_FOR;
    this->animation = this->animations;
  }

  uint8_t alpha = 0;

  if (this->timer >= DISPLAY_FOR) {
    this->timer = 0;
    this->initial_timer = thoom::Scene::instance->ticks;

    if (++this->animation >= this->animations) {
      thoom::Scene::instance->request_map(
          "maps/init");  // leave the splash screen
      return;
    }

    this->sprite->set_animation(this->animation);
    alpha = 255;
  } else if (this->timer > DISPLAY_FOR - 512) {
    alpha = THOOM_CLAMP((this->timer - (DISPLAY_FOR - 512)) / 2, 0, 255);
  } else if (this->timer < 512) {
    alpha = THOOM_CLAMP(256 - (this->timer / 2), 0, 255);
  }

  if (alpha > 0) {
    thoom::Renderer::instance->draw_rect(this->overlay, NULL,
                                         thoom::Colour{24, 24, 24, alpha},
                                         SDL_BLENDMODE_NONE);
  }

  if (this->animation == 4) {
    float perc = (float)this->timer / (float)DISPLAY_FOR;

    thoom::Renderer::instance->clear(this->tunnel, thoom::kMask);
    SDL_FRect tile_spr = {96.0f, 400.0f, 64.0f, 64.0f};
    SDL_FRect tile_dst = {float(THOOM_SCREEN_WIDTH / 2 - 32),
                          0.0f - perc * 128.0f, 64.0f,
                          (float)THOOM_SCREEN_HEIGHT + 128.0f};
    thoom::Renderer::instance->draw_texture_tiled(
        this->tunnel, this->sprite->texture, &tile_spr, 1.0f, &tile_dst);

    // ants

    float ant_x[] = {128.0f, 144.0f, 160.0f};
    float ant_y[] = {-40.0f, 40.0f, -12.0f, 7.0f, -30.0f};

    for (int which_ant = 0; which_ant < NUM_ANTS; which_ant++) {
      this->ant->update_frame();
      this->ant->set_animation(which_ant % 3);
      this->ant_src[which_ant] = this->ant->frame;

      this->ant_dst[which_ant].x = ant_x[which_ant % 3];
      this->ant_dst[which_ant].y = 0.0f - (perc * 256.0f) +
                                   (float)((which_ant / 3) * 64) +
                                   ant_y[which_ant % 5];
      this->ant_dst[which_ant].w = this->ant_dst[which_ant].h = 32.0f;

      if (this->ant_dst[which_ant].y < -32.0f)
        this->ant_dst[which_ant].y += (float)THOOM_SCREEN_HEIGHT + 32.0f;
      else if (this->ant_dst[which_ant].y < 0.0f) {
        float neg = this->ant_dst[which_ant].y;
        this->ant_dst[which_ant].y = 0.0f;
        this->ant_src[which_ant].y -= neg;
        this->ant_dst[which_ant].h = this->ant_src[which_ant].h = 32.0f + neg;
      }

      thoom::Renderer::instance->push_sprite(
          this->ant->tex_id, this->ant->texture, this->ant_src + which_ant,
          this->ant_dst + which_ant, 8);
    }

    // mice

    float cos_res = cosf(2.0f * perc * 2.0f * (float)M_PI);
    float sin_res = sinf(2.0f * perc * 2.0f * (float)M_PI);

    this->ched_dst.x = 160.0f - 16.0f + (20.0f * cos_res);
    this->ched_dst.y =
        (perc * (float)THOOM_SCREEN_HEIGHT) - 16.0f + (6.0f * sin_res);
    this->feta_dst.x = 160.0f - 16.0f - (20.0f * cos_res);
    this->feta_dst.y =
        (perc * (float)THOOM_SCREEN_HEIGHT) - 16.0f - (6.0f * sin_res);

    this->ched_src.x = sin_res < 0.0f ? 48.0f : 16.0f;
    this->feta_src.x = sin_res > 0.0f ? 48.0f : 16.0f;

    thoom::Renderer::instance->push_sprite(
        "", this->tunnel, &this->overlay_rect, &this->overlay_rect, 0);
    thoom::Renderer::instance->push_sprite(
        this->sprite->tex_id, this->sprite->texture, &this->ched_src,
        &this->ched_dst, 32);
    thoom::Renderer::instance->push_sprite(
        this->sprite->tex_id, this->sprite->texture, &this->feta_src,
        &this->feta_dst, 32);
  } else {
    this->sprite->interval_ms = this->sprite->animation > 2 ? 100 : 250;
    this->sprite->update_frame();

    float shake_x = 0.0f, shake_y = 0.0f;
    switch (this->animation) {
      case 3:
        shake_x = SDL_randf() + 4.0f;
        shake_y = SDL_randf() + 4.0f;

        break;
      case 5: {
        float perc = (float)this->timer / (float)DISPLAY_FOR;

        if (perc * 224.0f > 112.0f) {
          perc = 0.5f;
        }

        float cos_res = cosf((2.4f * perc * 2.0f * (float)M_PI) + M_PI);
        float sin_res = sinf((2.4f * perc * 2.0f * (float)M_PI) + M_PI);

        this->ched_dst.x = 160.0f - 16.0f + (20.0f * cos_res);
        this->ched_dst.y = (perc * 244.0f) - 16.0f + (6.0f * sin_res);
        this->feta_dst.x = 160.0f - 16.0f - (20.0f * cos_res);
        this->feta_dst.y = (perc * 244.0f) - 16.0f - (6.0f * sin_res);

        this->ched_src.x = sin_res < 0.0f ? 48.0f : 16.0f;
        this->feta_src.x = sin_res > 0.0f ? 48.0f : 16.0f;

        thoom::Renderer::instance->push_sprite(
            this->sprite->tex_id, this->sprite->texture, &this->ched_src,
            &this->ched_dst, 32);
        thoom::Renderer::instance->push_sprite(
            this->sprite->tex_id, this->sprite->texture, &this->feta_src,
            &this->feta_dst, 32);
      } break;
    }

    this->dst_rect.x = (float)(THOOM_SCREEN_WIDTH / 2) - 64.0f + shake_x;
    this->dst_rect.y = (float)(THOOM_SCREEN_HEIGHT / 2) - 64.0f + shake_y;

    thoom::Renderer::instance->push_sprite(
        this->sprite->tex_id, this->sprite->texture, &this->sprite->frame,
        &this->dst_rect, 0);
  }
  thoom::Renderer::instance->push_sprite("", this->overlay, &this->overlay_rect,
                                         &this->overlay_rect,
                                         2 * THOOM_SCREEN_HEIGHT);
}

thoom::Object* Splash::create_object(thoom::Scene* scene,
                                     const std::string& options) {
  return new Splash();
}
