#ifndef FONT_H
#define FONT_H

#include <map>

#include <SDL3/SDL.h>

#define NUM_DISPLAYABLE_CHARS ('~' - ' ' + 1)
#define FONT_SHEET_COLS       16

#define TEST_TEXT \
    " !\"#$%&'()*+,-./\n"\
    "0123456789:;<=>?@[\\]^_`|\n"\
    "AaBbCcDdEeFfGgHhIiJjKkLlMm\n"\
    "NnOoPpQqRrSsTtUuVvWwXxYyZz"

class Font {
public:
    Font(const char *font_path, int w, int h, int default_w, const std::unordered_map<char, int> &special_w);
    ~Font();

    SDL_FRect src_rect[NUM_DISPLAYABLE_CHARS];
    SDL_Texture *texture = nullptr;
};

#endif
