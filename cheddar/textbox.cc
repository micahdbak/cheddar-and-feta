#include "textbox.h"

#include "audio_playback.h"
#include "controller.h"
#include "game.h"
#include "renderer.h"

Textbox* textbox;

Textbox::Textbox(const std::string& text, const std::string& owner, int font,
                 int interval_ms) {
  thoom::Renderer* renderer = thoom::Renderer::instance;

  this->owner = owner;
  this->text = text;
  this->last_ticks = thoom::game->ticks;
  this->box_rect = {72.0f, 168.0f, 176.0f, 64.0f};
  this->text_rect = {80.0f, 176.0f, 160.0f, 48.0f};
  this->font = font;
  this->interval_ms = interval_ms;

  renderer->draw_ui_box(thoom::game->ui_box, BOX_CONTAINER, thoom::game->ui,
                        &this->box_rect);
}

Textbox::~Textbox() {
  // clear all ui
  thoom::Renderer::instance->clear(thoom::game->ui, thoom::kMask);
}

void Textbox::step() {
  if (this->done) return;

  thoom::Renderer* renderer = thoom::Renderer::instance;

  if (this->done_sentence) {
    if (thoom::local_controller.is_hit(thoom::Button::SELECT)) {
      this->done_sentence = false;
      this->running_text = "";
    }

    return;
  }

  if (thoom::local_controller.is_hit(thoom::Button::SELECT)) {
    int j;
    for (j = this->i; j < text.size() && text[j] != '\n'; j++) {
      this->running_text += text[j];
    }
    this->i = j;
    renderer->draw_rect(thoom::game->ui, &this->text_rect,
                        thoom::Colour{24, 24, 24}, SDL_BLENDMODE_NONE);
    renderer->draw_text(thoom::game->ui, thoom::game->fonts[this->font],
                        this->running_text, this->text_rect.x,
                        this->text_rect.y, this->text_rect.w,
                        thoom::Colour(24, 24, 24, 255));
    return;
  }

  if (thoom::game->ticks - this->last_ticks > this->interval_ms) {
    this->last_ticks = thoom::game->ticks;

    char c;
    bool space = false;
    do {
      c = i >= this->text.size() ? '\0' : this->text[i++];
      if (c != '\n' && c != '\0') this->running_text += c;
      if (c == ' ') space = true;
    } while (c == ' ');

    // don't blip if there was a space
    if (!space) {
      thoom::play_audio("sfx/blip.wav", 0.5f, 0.0f, 0.0f, true);
    }

    // display prompt arrow
    if (c == '\n' || c == '\0') {
      this->done = c == '\0';
      this->done_sentence = c == '\n';
      thoom::Font* font = thoom::game->fonts[this->font];
      SDL_FRect* prompt_src = font->src_rect + CHAR_TEXTBOX_NEXT - ' ';
      SDL_FRect prompt_dst = {
          this->text_rect.x + this->text_rect.w - prompt_src->w,
          this->text_rect.y + this->text_rect.h - prompt_src->h, prompt_src->w,
          prompt_src->h};
      renderer->draw_texture(thoom::game->ui, font->texture, prompt_src,
                             &prompt_dst);
      return;
    }

    renderer->draw_rect(thoom::game->ui, &this->text_rect,
                        thoom::Colour{24, 24, 24}, SDL_BLENDMODE_NONE);
    renderer->draw_text(thoom::game->ui, thoom::game->fonts[this->font],
                        this->running_text, this->text_rect.x,
                        this->text_rect.y, this->text_rect.w,
                        thoom::Colour(24, 24, 24, 255));
  }
}
