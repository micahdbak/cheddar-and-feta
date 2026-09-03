#include "controller.h"

#include <iostream>

#include "SDL3/SDL_keycode.h"
#include "game.h"

std::map<SDL_Keycode, Button> ktobutton_map = {
    {SDLK_ESCAPE, Button::MENU},      {SDLK_UP, Button::UP},
    {SDLK_RIGHT, Button::RIGHT},      {SDLK_DOWN, Button::DOWN},
    {SDLK_LEFT, Button::LEFT},        {SDLK_RETURN, Button::SELECT},
    {SDLK_BACKSPACE, Button::CANCEL}, {SDLK_T, Button::TOSS},
    {SDLK_SPACE, Button::ATTACK},     {SDLK_RSHIFT, Button::DANCE},
};

std::map<Button, SDL_Keycode> btokeycode_map = {
    {Button::MENU, SDLK_ESCAPE},      {Button::UP, SDLK_UP},
    {Button::RIGHT, SDLK_RIGHT},      {Button::DOWN, SDLK_DOWN},
    {Button::LEFT, SDLK_LEFT},        {Button::SELECT, SDLK_RETURN},
    {Button::CANCEL, SDLK_BACKSPACE}, {Button::TOSS, SDLK_T},
    {Button::ATTACK, SDLK_SPACE},     {Button::DANCE, SDLK_RSHIFT},
};

std::map<Button, std::string> btostring_map = {
    {Button::MENU, "Menu"},     {Button::UP, "Up"},
    {Button::RIGHT, "Right"},   {Button::DOWN, "Down"},
    {Button::LEFT, "Left"},     {Button::SELECT, "Select"},
    {Button::CANCEL, "Cancel"}, {Button::DANCE, "Dance"},
    {Button::ATTACK, "Attack"}, {Button::TOSS, "Toss"},
};

static Button _keycode_to_button(SDL_Keycode keycode) {
  Button button;

  if (ktobutton_map.find(keycode) == ktobutton_map.end()) {
    return Button::NULL_BUTTON;
  }

  return ktobutton_map[keycode];
}

void Controller::clear_hits() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    this->is_hit_map[i] = false;
  }
  this->c = NO_CHAR;
  this->digit = -1;
}

void Controller::clear_all() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    this->is_hit_map[i] = false;
    this->is_down_map[i] = false;
  }
  this->c = NO_CHAR;
  this->digit = -1;
}

bool Controller::is_hit(Button button) {
  if (this->is_hit_map[button]) {
    this->is_hit_map[button] = false;
    return true;
  }

  return false;
}

bool Controller::is_down(Button button) { return this->is_down_map[button]; }

void Controller::handle_button_hit(Button button) {
  this->is_hit_map[button] = true;
}

void Controller::handle_button_down(Button button) {
  this->is_down_map[button] = true;
}

void Controller::handle_button_up(Button button) {
  this->is_down_map[button] = false;
}

void Controller::handle_key_down(SDL_Keycode keycode) {
  if (keycode >= ' ' && keycode <= '~') {
    this->c = char(keycode);
  }

  Button button = NULL_BUTTON;

  if (keycode >= (uint32_t)'0' && keycode <= (uint32_t)'9') {
    // digits added to the digit string must be enterred within <1s
    if (game->ticks - this->last_digit_hit > 1000) {
      this->digit_string.clear();
      this->last_digit_hit = game->ticks;
    }

    this->digit = keycode - '0';
    this->digit_string.push_back(this->digit);

    if (this->digit_string.size() > 4) {
      this->digit_string[0] = this->digit_string[1];
      this->digit_string[1] = this->digit_string[2];
      this->digit_string[2] = this->digit_string[3];
      this->digit_string[3] = this->digit_string[4];
      this->digit_string.pop_back();
    }

    button = DIGIT;
  } else {
    button = _keycode_to_button(keycode);
  }

  if (button == NULL_BUTTON) {
    return;
  }

  this->is_hit_map[button] = true;
  this->is_down_map[button] = true;
}

void Controller::handle_key_up(SDL_Keycode keycode) {
  Button button = NULL_BUTTON;

  if (keycode >= '0' && keycode <= '9') {
    button = DIGIT;
  } else {
    button = _keycode_to_button(keycode);
  }

  if (button == NULL_BUTTON) {
    return;
  }

  this->is_down_map[button] = false;
}

bool Controller::cheat_code(int a, int b, int c, int d) {
  return this->digit_string.size() == 4 && this->digit_string[0] == a &&
         this->digit_string[1] == b && this->digit_string[2] == c &&
         this->digit_string[3] == d;
}

int Controller::cheat_last(int a, int b, int c) {
  if (this->digit_string.size() == 4 && this->digit_string[0] == a &&
      this->digit_string[1] == b && this->digit_string[2] == c)
    return this->digit_string[3];

  return -1;
}