#include "textbox.h"

#include <string>

#include "audio.h"
#include "colour.h"
#include "constants.h"
#include "controller.h"
#include "font.h"
#include "hud.h"
#include "input.h"
#include "renderer.h"
#include "scene.h"

Textbox* textbox;

Textbox::Textbox(const std::string& text, const std::string& owner, int font,
                 int interval_ms) {
  this->owner = owner;
  this->text = text;
  this->last_ticks = thoom::Scene::instance->ticks;
  this->box_rect = {72.0f, 168.0f, 176.0f, 64.0f};
  this->text_rect = {80.0f, 176.0f, 160.0f, 48.0f};
  this->font = font;
  this->interval_ms = interval_ms;

  thoom::Renderer::instance->draw_ui_box(
      UI_BOX_CONTAINER, thoom::Hud::instance->ui, &this->box_rect);
}

Textbox::~Textbox() {
  // clear all ui
  thoom::Renderer::instance->clear(thoom::Hud::instance->ui, thoom::kMask);
}

void Textbox::step() {
  if (this->done) return;

  if (this->done_sentence) {
    if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
      this->done_sentence = false;
      this->running_text = "";
    }

    return;
  }

  if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
    int j;
    for (j = this->i; j < text.size() && text[j] != '\n'; j++) {
      this->running_text += text[j];
    }
    this->i = j;
    thoom::Renderer::instance->draw_rect(thoom::Hud::instance->ui,
                                         &this->text_rect, kBackground,
                                         SDL_BLENDMODE_NONE);
    thoom::Renderer::instance->draw_text(
        thoom::Hud::instance->ui, this->font, this->running_text,
        this->text_rect.x, this->text_rect.y, this->text_rect.w, kBackground);
    return;
  }

  if (thoom::Scene::instance->ticks - this->last_ticks > this->interval_ms) {
    this->last_ticks = thoom::Scene::instance->ticks;

    char c;
    bool space = false;
    do {
      c = i >= this->text.size() ? '\0' : this->text[i++];
      if (c != '\n' && c != '\0') this->running_text += c;
      if (c == ' ') space = true;
    } while (c == ' ');

    // don't blip if there was a space
    if (!space) {
      thoom::Audio::instance->play("sfx/blip.wav", 0.5f, 0.0f, 0.0f, true);
    }

    // display prompt arrow
    if (c == '\n' || c == '\0') {
      this->done = c == '\0';
      this->done_sentence = c == '\n';
      thoom::Font* font = thoom::Renderer::instance->fonts[this->font];
      SDL_FRect* prompt_src = font->src_rect + CHAR_TEXTBOX_NEXT - ' ';
      SDL_FRect prompt_dst = {
          this->text_rect.x + this->text_rect.w - prompt_src->w,
          this->text_rect.y + this->text_rect.h - prompt_src->h, prompt_src->w,
          prompt_src->h};
      thoom::Renderer::instance->draw_texture(
          thoom::Hud::instance->ui, font->texture, prompt_src, &prompt_dst);
      return;
    }

    thoom::Renderer::instance->draw_rect(thoom::Hud::instance->ui,
                                         &this->text_rect, kBackground,
                                         SDL_BLENDMODE_NONE);
    thoom::Renderer::instance->draw_text(
        thoom::Hud::instance->ui, this->font, this->running_text,
        this->text_rect.x, this->text_rect.y, this->text_rect.w, kBackground);
  }
}
