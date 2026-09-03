#include "font.h"

#include <iostream>

#include "game.h"

void Font::load_fonts(std::vector<Font*>& fonts) {
  // MONO_FONT = 0
  fonts.push_back(new Font("fonts/mono.bmp", 6, 10, 6, {}));

  // SMALL_FONT = 1
  fonts.push_back(new Font(
      "fonts/small.bmp", 6, 7, 4,
      {{' ', 3}, {'!', 2}, {'#', 6}, {'%', 5}, {'&', 5}, {'\'', 3}, {'*', 6},
       {',', 3}, {'.', 2}, {':', 2}, {';', 3}, {'<', 6}, {'>', 6},  {'@', 5},
       {'M', 6}, {'N', 5}, {'W', 6}, {'^', 6}, {'`', 3}, {'i', 2},  {'j', 3},
       {'l', 2}, {'m', 6}, {'w', 6}, {'{', 6}, {'|', 6}, {'}', 6},  {'~', 7}}));

  // SM_BOLD_FONT = 2
  fonts.push_back(new Font(
      "fonts/small_bold.bmp", 6, 7, 5,
      {{' ', 3}, {'!', 2}, {'"', 4}, {'#', 6}, {'$', 4}, {'\'', 3}, {'(', 4},
       {')', 4}, {'*', 6}, {'+', 4}, {',', 3}, {'.', 2}, {':', 2},  {';', 3},
       {'<', 6}, {'>', 6}, {'M', 6}, {'N', 6}, {'W', 6}, {'^', 6},  {'`', 3},
       {'i', 3}, {'j', 4}, {'l', 3}, {'m', 6}, {'w', 6}, {'x', 6},  {'{', 6},
       {'|', 6}, {'}', 6}, {'~', 7}}));

  // DEFAULT_FONT = 3
  fonts.push_back(new Font(
      "fonts/default.bmp", 6, 10, 5,
      {{' ', 3}, {'!', 2}, {'"', 4}, {'#', 6}, {'\'', 3}, {'(', 4}, {')', 4},
       {'*', 4}, {'+', 6}, {',', 3}, {'.', 2}, {'1', 4},  {':', 2}, {';', 3},
       {'I', 4}, {'J', 4}, {'M', 6}, {'V', 6}, {'W', 6},  {'X', 6}, {'[', 4},
       {']', 4}, {'^', 4}, {'`', 3}, {'i', 2}, {'j', 3},  {'l', 2}, {'m', 6},
       {'v', 6}, {'w', 6}, {'x', 6}, {'{', 6}, {'|', 6},  {'}', 6}}));

  // BOLD_FONT = 4
  fonts.push_back(new Font(
      "fonts/bold.bmp", 7, 10, 6,
      {{' ', 4}, {'!', 2}, {'"', 4},  {'$', 5}, {'%', 5}, {'&', 5}, {'\'', 3},
       {'(', 4}, {')', 4}, {'*', 4},  {',', 3}, {'.', 2}, {'/', 5}, {'1', 5},
       {':', 2}, {';', 3}, {'?', 5},  {'I', 5}, {'J', 5}, {'M', 7}, {'N', 7},
       {'T', 5}, {'[', 4}, {'\\', 5}, {']', 4}, {'^', 4}, {'`', 3}, {'i', 5},
       {'j', 4}, {'l', 5}, {'m', 7},  {'w', 7}}));

  // CODE_FONT = 5
  fonts.push_back(new Font("fonts/code.bmp", 6, 10, 6, {}));

  // CODE_GRAY_FONT = 6
  fonts.push_back(new Font("fonts/code_gray.bmp", 6, 10, 6, {}));

  // TITLE_FONT = 7
  fonts.push_back(new Font(
      "fonts/title.bmp", 8, 13, 6,
      {{' ', 4}, {'\'', 4}, {',', 4}, {'A', 8}, {'B', 7}, {'C', 7}, {'D', 7},
       {'G', 7}, {'J', 7},  {'K', 7}, {'M', 8}, {'N', 7}, {'O', 7}, {'P', 7},
       {'Q', 7}, {'R', 7},  {'S', 7}, {'U', 7}, {'V', 7}, {'W', 8}, {'X', 8},
       {'Y', 8}, {'Z', 7},  {'f', 5}, {'i', 4}, {'j', 3}, {'l', 4}, {'m', 8},
       {'r', 5}, {'t', 5},  {'w', 8}}));

  // CONTROLS_FONT = 6
  fonts.push_back(new Font("fonts/controls.bmp", 8, 16, 8, {{'/', 5}}));
}

Font::Font(const char* font_path, int w, int h, int default_w,
           const std::unordered_map<char, int>& special_w) {
  SDL_Surface* font_surface = SDL_LoadBMP(font_path);
  if (font_surface == nullptr) {
    std::cerr << "Font::Font error: '" << font_path << "' does not exist."
              << std::endl;
    exit(1);
  }
  this->texture = SDL_CreateTextureFromSurface(renderer, font_surface);
  SDL_SetTextureScaleMode(this->texture, SDL_SCALEMODE_NEAREST);
  SDL_DestroySurface(font_surface);

  // create all src_rect's
  for (int i = 0; i < NUM_DISPLAYABLE_CHARS; i++) {
    this->src_rect[i].x = float((i % FONT_SHEET_COLS) * w);
    this->src_rect[i].y = float((i / FONT_SHEET_COLS) * h);
    this->src_rect[i].w = float(default_w);
    this->src_rect[i].h = float(h);

    // see if this character has a special width
    char c = char(i) + ' ';
    if (special_w.find(c) != special_w.end()) {
      this->src_rect[i].w = float(special_w.at(c));
    }
  }
}

Font::~Font() {
  if (this->texture != nullptr) {
    SDL_DestroyTexture(this->texture);
    this->texture = nullptr;
  }
}

int Font::text_width(const std::string& text) {
  int w = 0;

  for (char c : text) {
    int i = c - ' ';
    if (i < 0 || i >= NUM_DISPLAYABLE_CHARS) continue;
    w += int(this->src_rect[i].w + 0.1f);
  }

  return w;
}
