#include "game.h"
#include "controller.h"
#include "textbox.h"
#include "audio_playback.h"

Textbox *textbox;

Textbox::Textbox(const std::string &text, const std::string &owner, int font, int interval_ms) {
    this->owner = owner;
    this->text = text;
    this->last_ticks = game->ticks;
    this->box_rect = { 72.0f, 168.0f, 176.0f, 64.0f };
    this->text_rect = { 80.0f, 176.0f, 160.0f, 48.0f };
    this->font = font;
    this->interval_ms = interval_ms;
    game->draw_ui_box(game->ui, BOX_CONTAINER, &this->box_rect);
}

Textbox::~Textbox() {
    // clear all ui
    game->draw_rect(game->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
}

void Textbox::step() {
    if (this->done)
        return;

    if (this->done_sentence) {
        if (local_controller.is_hit(Button::SELECT)) {
            this->done_sentence = false;
            this->running_text = "";
        }

        return;
    }

    if (local_controller.is_hit(Button::SELECT)) {
        int j;
        for (j = this->i; j < text.size() && text[j] != '\n'; j++) {
            this->running_text += text[j];
        }
        this->i = j;
        game->draw_rect(game->ui, &this->text_rect, 24, 24, 24, 255, SDL_BLENDMODE_NONE);
        game->draw_text(game->ui, this->running_text, this->font, this->text_rect.x, this->text_rect.y, this->text_rect.w);
        return;
    }

    if (game->ticks - this->last_ticks > this->interval_ms) {
        this->last_ticks = game->ticks;
        
        char c;
        bool space = false;
        do {
            c = i >= this->text.size() ? '\0' : this->text[i++];
            if (c != '\n' && c != '\0')
                this->running_text += c;
            if (c == ' ')
                space = true;
        } while (c == ' ');

        // don't blip if there was a space
        if (!space) {
            play_audio("sfx/blip.wav", 0.5f, 0.0f, 0.0f);
        }

        // display prompt arrow
        if (c == '\n' || c == '\0') {
            this->done = c == '\0';
            this->done_sentence = c == '\n';
            SDL_SetRenderTarget(renderer, game->ui);
            SDL_FRect *prompt_src = game->fonts[this->font]->src_rect + CHAR_TEXTBOX_NEXT - ' ';
            SDL_FRect prompt_dst = {
                this->text_rect.x + this->text_rect.w - prompt_src->w,
                this->text_rect.y + this->text_rect.h - prompt_src->h,
                prompt_src->w, prompt_src->h
            };
            SDL_RenderTexture(renderer, game->fonts[this->font]->texture, prompt_src, &prompt_dst);
            SDL_SetRenderTarget(renderer, game->screen);
            return;
        }

        game->draw_rect(game->ui, &this->text_rect, 24, 24, 24, 255, SDL_BLENDMODE_NONE);
        game->draw_text(game->ui, this->running_text, this->font, this->text_rect.x, this->text_rect.y, this->text_rect.w);
    }
}
