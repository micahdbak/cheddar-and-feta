#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <string>

#include <SDL3/SDL.h>

#define LOREM_IPSUM \
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"\
    "Donec vehicula venenatis arcu quis blandit.\n"\
    "Suspendisse sagittis risus vitae euismod eleifend.\n"\
    "Quisque mauris felis, scelerisque nec rutrum sit amet, molestie ut quam.\n"\
    "Suspendisse non blandit lorem, quis mollis felis.\n"\
    "Vestibulum vitae ante elementum libero cursus pharetra in non nibh.\n"\
    "Curabitur maximus sem arcu, ac convallis nisl varius vitae.\n"\
    "Fusce quis magna et orci fringilla viverra.\n"\
    "Praesent accumsan leo dictum egestas luctus.\n"\
    "Mauris nibh ligula, porta quis magna id, egestas rutrum eros.\n"\
    "That will be 7$. You have 15% health points. Cheddar & Feta.\n"\
    "Thanks for reading - Cheddar n' Feta!"

class Textbox {
public:
    Textbox(const std::string &text, const std::string &owner, int font, int interval_ms);
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

extern Textbox *textbox;

#endif
