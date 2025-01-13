#include "keyboard.h"

Keyboard::Keyboard() {}

Keyboard::~Keyboard() {}

void Keyboard::clear_hits() {
    for (auto &key : this->is_hit_map) {
        // unset all hits that were true
        if (this->is_hit_map.at(key.first)) {
            this->is_hit_map[key.first] = false;
        }
    }
}

bool Keyboard::is_hit(SDL_Keycode key) {
    if (!this->is_hit_map.contains(key)) {
        return false;
    }

    return this->is_hit_map.at(key);
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
}

void Keyboard::handle_up(SDL_Keycode key) {
    this->is_down_map[key] = false;
}
