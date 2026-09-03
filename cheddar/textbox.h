#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <SDL3/SDL.h>

#include <string>

class Textbox {
 public:
  Textbox(const std::string& text, const std::string& owner, int font,
          int interval_ms);
  ~Textbox();

  void step();

  std::string owner;
  bool done_sentence = false, done = false;

 private:
  SDL_FRect box_rect, text_rect;
  int i = 0, font, interval_ms;
  Uint64 last_ticks;
  std::string text, running_text;
};

extern Textbox* textbox;

#endif
