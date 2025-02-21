#include "game.h"
#include "controller.h"

#include <iostream>

static std::unordered_map<SDL_Keycode, Button> _keycode_map = {
    { SDLK_ESCAPE, PAUSE },
    { SDLK_TAB, MENU },
    { SDLK_LSHIFT, L2 },
    { SDLK_RSHIFT, R2 },
    { SDLK_P, L1 },
    { SDLK_C, R1 },
    { SDLK_RETURN, PRIMARY },
    { SDLK_BACKSPACE, SECONDARY },
    { SDLK_SPACE, ACTION1 },
    { SDLK_T, ACTION2 },
    { SDLK_UP, UP },
    { SDLK_RIGHT, RIGHT },
    { SDLK_DOWN, DOWN },
    { SDLK_LEFT, LEFT },
    { SDLK_W, UP },
    { SDLK_D, RIGHT },
    { SDLK_S, DOWN },
    { SDLK_A, LEFT },
};

static std::unordered_map<SDL_GamepadButton, Button> _gamepad_map = {
    { SDL_GAMEPAD_BUTTON_SOUTH, PRIMARY },
    { SDL_GAMEPAD_BUTTON_EAST, SECONDARY },
    { SDL_GAMEPAD_BUTTON_WEST, ACTION1 },
    { SDL_GAMEPAD_BUTTON_NORTH, ACTION2 },
    { SDL_GAMEPAD_BUTTON_BACK, PAUSE },
    { SDL_GAMEPAD_BUTTON_START, MENU },
    { SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, L1 },
    { SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, R1 },
    { SDL_GAMEPAD_BUTTON_DPAD_UP, UP },
    { SDL_GAMEPAD_BUTTON_DPAD_RIGHT, RIGHT },
    { SDL_GAMEPAD_BUTTON_DPAD_DOWN, DOWN },
    { SDL_GAMEPAD_BUTTON_DPAD_LEFT, LEFT }
};

Controller::Controller() {
    for (int i = 0; i < int(NUM_BUTTONS); i++) {
        this->is_down_map[(Button)i] = false;
    }
}

void Controller::clear_hits() {
    this->is_hit_map.clear();
    this->c = NO_CHAR;
}

bool Controller::is_hit(Button button) {
    if (this->is_hit_map.contains(button) && this->is_hit_map[button]) {
        this->is_hit_map[button] = false;
        return true;
    } else {
        return false;
    }
}

bool Controller::is_down(Button button) {
    return this->is_down_map.at(button);
}

void Controller::handle_button_down(Button button) {
    this->is_hit_map[button] = true;
    this->is_down_map[button] = true;
}

void Controller::handle_button_up(Button button) {
    this->is_down_map[button] = false;
}

void Controller::handle_key_down(SDL_Keycode keycode) {
    if (keycode >= ' ' && keycode <= '~') {
        if (this->is_down(_keycode_map[SDLK_LSHIFT]) && keycode >= 'a' && keycode <= 'z')
            this->c = char(keycode - 32);
        else
            this->c = char(keycode);
    }

    if (!_keycode_map.contains(keycode)) {
        return;
    }

    Button button = _keycode_map[keycode];
    this->is_hit_map[button] = true;
    this->is_down_map[button] = true;
}

void Controller::handle_key_up(SDL_Keycode keycode) {
    if (!_keycode_map.contains(keycode)) {
        return;
    }

    Button button = _keycode_map[keycode];
    this->is_down_map[button] = false;
}

void Controller::handle_gamepad_down(SDL_GamepadButton gamepad_button) {
    if (!_gamepad_map.contains(gamepad_button)) {
        return;
    }

    Button button = _gamepad_map[gamepad_button];
    this->is_hit_map[button] = true;
    this->is_down_map[button] = true;
}

void Controller::handle_gamepad_up(SDL_GamepadButton gamepad_button) {
    if (!_gamepad_map.contains(gamepad_button)) {
        return;
    }

    Button button = _gamepad_map[gamepad_button];
    this->is_down_map[button] = false;
}

void Controller::handle_gamepad_axis(SDL_GamepadAxis axis, Sint16 value) {
    switch (axis) {
    // "fake" axis
    case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
        if (abs(value) < 8000) {
            this->is_down_map[L2] = false;
        } else if (this->is_down_map[L2] == false) {
            this->is_hit_map[L2] = true;
            this->is_down_map[L2] = false;
        }
        return;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
        if (abs(value) < 8000) {
            this->is_down_map[R2] = false;
        } else if (this->is_down_map[R2] == false) {
            this->is_hit_map[R2] = true;
            this->is_down_map[R2] = false;
        }
        return;

    // "real" axis
    case SDL_GAMEPAD_AXIS_LEFTX:
    case SDL_GAMEPAD_AXIS_RIGHTX:
        this->stick_x = value;
        break;

    case SDL_GAMEPAD_AXIS_LEFTY:
    case SDL_GAMEPAD_AXIS_RIGHTY:
        this->stick_y = value;
        break;

    default: return;
    }

    float distance = distance_between_points(0, 0, float(this->stick_x), float(this->stick_y));
    if (distance < 8000.0f) {
        this->is_down_map[UP] = this->is_down_map[RIGHT] = this->is_down_map[DOWN] = this->is_down_map[LEFT] = false;
        return;
    }

    int x_dir, y_dir;
    dir_to_point(0, 0, float(this->stick_x), float(this->stick_y), &x_dir, &y_dir);

    if (x_dir == 0) {
        this->is_down_map[RIGHT] = this->is_down_map[LEFT] = false;
    } else if (x_dir > 0) {
        this->is_down_map[LEFT] = false;
        if (!this->is_down_map[RIGHT]) {
            this->is_hit_map[RIGHT] = true;
            this->is_down_map[RIGHT] = true;
        }
    } else {
        this->is_down_map[RIGHT] = false;
        if (!this->is_down_map[LEFT]) {
            this->is_hit_map[LEFT] = true;
            this->is_down_map[LEFT] = true;
        }
    }

    if (y_dir == 0) {
        this->is_down_map[DOWN] = this->is_down_map[UP] = false;
    } else if (y_dir > 0) {
        this->is_down_map[UP] = false;
        if (!this->is_down_map[DOWN]) {
            this->is_hit_map[DOWN] = true;
            this->is_down_map[DOWN] = true;
        }
    } else {
        this->is_down_map[DOWN] = false;
        if (!this->is_down_map[UP]) {
            this->is_hit_map[UP] = true;
            this->is_down_map[UP] = true;
        }
    }
}
