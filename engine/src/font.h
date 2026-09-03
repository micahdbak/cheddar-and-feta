#ifndef FONT_H
#define FONT_H

#include <SDL3/SDL.h>

#include <string>
#include <unordered_map>
#include <vector>

#define NUM_DISPLAYABLE_CHARS ('~' - ' ' + 1)
#define FONT_SHEET_COLS 16

// fonts
#define MONO_FONT 0
#define SMALL_FONT 1
#define SM_BOLD_FONT 2
#define DEFAULT_FONT 3
#define BOLD_FONT 4
#define CODE_FONT 5
#define CODE_GRAY_FONT 6
#define TITLE_FONT 7
#define CONTROLS_FONT 8
#define NUM_FONTS 9

// special chars in SMALL_FONT
#define CHAR_EQUIP '{'
#define CHAR_TEXTBOX_NEXT '|'
#define CHAR_DOWN_ARROW '}'

#define TEST_TEXT                \
  " !\"#$%&'()*+,-./\n"          \
  "0123456789:;<=>?@[\\]^_`|\n"  \
  "AaBbCcDdEeFfGgHhIiJjKkLlMm\n" \
  "NnOoPpQqRrSsTtUuVvWwXxYyZz"

#define LOREM_IPSUM                                                            \
  "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"                 \
  "Donec vehicula venenatis arcu quis blandit.\n"                              \
  "Suspendisse sagittis risus vitae euismod eleifend.\n"                       \
  "Quisque mauris felis, scelerisque nec rutrum sit amet, molestie ut quam.\n" \
  "Suspendisse non blandit lorem, quis mollis felis.\n"                        \
  "Vestibulum vitae ante elementum libero cursus pharetra in non nibh.\n"      \
  "Curabitur maximus sem arcu, ac convallis nisl varius vitae.\n"              \
  "Fusce quis magna et orci fringilla viverra.\n"                              \
  "Praesent accumsan leo dictum egestas luctus.\n"                             \
  "Mauris nibh ligula, porta quis magna id, egestas rutrum eros.\n"            \
  "That will be 7$. You have 15% health points. Cheddar & Feta.\n"             \
  "Thanks for reading - Cheddar n' Feta!"

class Font {
 public:
  static void load_fonts(std::vector<Font*>& fonts);

  Font(const char* font_path, int w, int h, int default_w,
       const std::unordered_map<char, int>& special_w);
  ~Font();

  int text_width(const std::string& text);

  SDL_FRect src_rect[NUM_DISPLAYABLE_CHARS];
  SDL_Texture* texture = nullptr;
};

#endif
