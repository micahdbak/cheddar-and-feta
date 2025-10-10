#include <cmath>

#include "splash.h"
#include "game.h"
#include "controller.h"

Splash::Splash() {
    this->overlay = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    this->tunnel = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

    this->overlay_rect.x = this->overlay_rect.y = 0.0f;
    this->overlay_rect.w = SCREEN_WIDTH;
    this->overlay_rect.h = SCREEN_HEIGHT;

    this->sprite = new Sprite("sprites/splash.bmp", 128, 96, 250);
    this->sprite->set_frame(this->animation);

    this->ched_src = { 16.0f, 400.0f, 32.0f, 32.0f };
    this->ched_dst.w = this->ched_dst.h = 32.0f;
    this->feta_src = { 16.0f, 432.0f, 32.0f, 32.0f };
    this->feta_dst.w = this->feta_dst.h = 32.0f;

    this->dst_rect.w = 128.0f;
    this->dst_rect.h = 96.0f;

    this->initial_timer = game->ticks;

    this->animations = 7;

    game->bg_r = 24;
    game->bg_g = 24;
    game->bg_b = 24;
    game->corner_x = 0;
    game->corner_y = 0;
}

Splash::~Splash() {
    delete this->sprite;
    SDL_DestroyTexture(this->overlay);
    SDL_DestroyTexture(this->tunnel);

    game->bg_r = DEFAULT_BG_R;
    game->bg_g = DEFAULT_BG_G;
    game->bg_b = DEFAULT_BG_B;
}

#define DISPLAY_FOR 4000

void Splash::step() {
    this->timer = game->ticks - this->initial_timer;

    // skip splash
    if (local_controller.is_hit(Button::SELECT)) {
        this->timer = DISPLAY_FOR;
        this->animation = this->animations;
    }

    uint8_t alpha = 0;

    if (this->timer >= DISPLAY_FOR) {
        this->timer = 0;
        this->initial_timer = game->ticks;

        if (++this->animation >= this->animations) {
            game->map = "maps/init"; // leave the splash screen
            return;
        }

        this->sprite->set_animation(this->animation);
        alpha = 255;
    } else if (this->timer > DISPLAY_FOR - 512) {
        alpha = cnf_clamp((this->timer - (DISPLAY_FOR - 512)) / 2, 0, 255);
    } else if (this->timer < 512) {
        alpha = cnf_clamp(256 - (this->timer / 2), 0, 255);
    }

    if (alpha > 0) {
        SDL_SetRenderTarget(renderer, this->overlay);
        SDL_SetRenderDrawColor(renderer, 24, 24, 24, alpha);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderFillRect(renderer, NULL);
        SDL_SetRenderTarget(renderer, game->screen);
    }

    if (this->animation == 4) {
        float perc = (float)this->timer / (float)DISPLAY_FOR;

        SDL_SetRenderTarget(renderer, this->tunnel);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderFillRect(renderer, NULL);
        SDL_FRect tile_spr = {96.0f, 400.0f, 64.0f, 64.0f};
        SDL_FRect tile_dst = {float(SCREEN_WIDTH/2 - 32), 0.0f - perc * 128.0f, 64.0f, (float)SCREEN_HEIGHT + 128.0f};
        SDL_RenderTextureTiled(renderer, this->sprite->texture, &tile_spr, 1.0f, &tile_dst);
        SDL_SetRenderTarget(renderer, game->screen);

        float cos_res = cosf(2.0f * perc * 2.0f * (float)M_PI);
        float sin_res = sinf(2.0f * perc * 2.0f * (float)M_PI);

        this->ched_dst.x = 160.0f - 16.0f + (20.0f * cos_res);
        this->ched_dst.y = (perc * (float)SCREEN_HEIGHT) - 16.0f + (6.0f * sin_res);
        this->feta_dst.x = 160.0f - 16.0f - (20.0f * cos_res);
        this->feta_dst.y = (perc * (float)SCREEN_HEIGHT) - 16.0f - (6.0f * sin_res);

        this->ched_src.x = sin_res < 0.0f ? 48.0f : 16.0f;
        this->feta_src.x = sin_res > 0.0f ? 48.0f : 16.0f;

        game->push_sprite("", this->tunnel, &this->overlay_rect, &this->overlay_rect, 0);
        game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->ched_src, &this->ched_dst, 32);
        game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->feta_src, &this->feta_dst, 32);
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

            float cos_res = cosf((2.0f * perc * 2.0f * (float)M_PI) + M_PI);
            float sin_res = sinf((2.0f * perc * 2.0f * (float)M_PI) + M_PI);
            bool settled = perc * 224.0f > 112.0f;

            if (settled) {
                this->ched_dst.x = 160.0f - 16.0f - 20.0f;
                this->ched_dst.y = 112.0f - 16.0f;
                this->feta_dst.x = 160.0f - 16.0f + 20.0f;
                this->feta_dst.y = 112.0f - 16.0f;
            } else {
                this->ched_dst.x = 160.0f - 16.0f + (20.0f * cos_res);
                this->ched_dst.y = (perc * 224.0f) - 16.0f + (6.0f * sin_res);
                this->feta_dst.x = 160.0f - 16.0f - (20.0f * cos_res);
                this->feta_dst.y = (perc * 224.0f) - 16.0f - (6.0f * sin_res);

                this->ched_src.x = sin_res < 0.0f ? 48.0f : 16.0f;
                this->feta_src.x = sin_res > 0.0f ? 48.0f : 16.0f;
            }

            game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->ched_src, &this->ched_dst, 32);
            game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->feta_src, &this->feta_dst, 32);
        } break;
        }

        this->dst_rect.x = (float)(SCREEN_WIDTH / 2) - 64.0f + shake_x;
        this->dst_rect.y = (float)(SCREEN_HEIGHT / 2) - 64.0f + shake_y;

        game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 0);
    }
    game->push_sprite("", this->overlay, &this->overlay_rect, &this->overlay_rect, 2*SCREEN_HEIGHT);
}
