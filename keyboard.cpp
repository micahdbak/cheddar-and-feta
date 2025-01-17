#include <iostream>

#include "keyboard.h"

Keyboard::Keyboard() {}

Keyboard::~Keyboard() {}

void Keyboard::clear_hits() {
    this->is_hit_map.clear();
    this->c = NO_CHAR;
}

bool Keyboard::is_hit(SDL_Keycode key) {
    return this->is_hit_map.contains(key);
}

bool Keyboard::is_down(SDL_Keycode key) {
    if (!this->is_down_map.contains(key)) {
        return false;
    }

    return this->is_down_map.at(key);
}

void Keyboard::handle_down(SDL_Keycode key) {
    this->is_hit_map[key] = true;
    this->is_down_map[key] = true;

    if (key >= ' ' && key <= '~') {
        if (this->is_down(SDLK_LSHIFT) && key >= 'a' && key <= 'z')
            this->c = char(key - 32);
        else
            this->c = char(key);
    }
}

void Keyboard::handle_up(SDL_Keycode key) {
    this->is_down_map[key] = false;
}
