#include "SDL3/SDL_keycode.h"
#include "game.h"
#include "controller.h"

#include <iostream>

std::map<SDL_Keycode, Button> ktobutton_map = {
    { SDLK_ESCAPE, Button::MENU },
    { SDLK_UP, Button::UP },
    { SDLK_RIGHT, Button::RIGHT },
    { SDLK_DOWN, Button::DOWN },
    { SDLK_LEFT, Button::LEFT },
    { SDLK_RETURN, Button::SELECT },
    { SDLK_BACKSPACE, Button::CANCEL },
    { SDLK_Q, Button::CYCLE_LEFT },
    { SDLK_E, Button::CYCLE_RIGHT },
    { SDLK_W, Button::TOSS },
    { SDLK_R, Button::RUN },
    { SDLK_SPACE, Button::ATTACK },
    { SDLK_RSHIFT, Button::DANCE },
};

std::map<Button, SDL_Keycode> btokeycode_map = {
    { Button::MENU, SDLK_ESCAPE },
    { Button::UP, SDLK_UP },
    { Button::RIGHT, SDLK_RIGHT },
    { Button::DOWN, SDLK_DOWN },
    { Button::LEFT, SDLK_LEFT },
    { Button::SELECT, SDLK_RETURN },
    { Button::CANCEL, SDLK_BACKSPACE },
    { Button::CYCLE_LEFT, SDLK_Q },
    { Button::CYCLE_RIGHT, SDLK_E },
    { Button::TOSS, SDLK_W },
    { Button::RUN, SDLK_R },
    { Button::ATTACK, SDLK_SPACE },
    { Button::DANCE, SDLK_RSHIFT },
};

std::map<Button, std::string> btostring_map = {
    { Button::MENU, "Menu" },
    { Button::UP, "Up" },
    { Button::RIGHT, "Right" },
    { Button::DOWN, "Down" },
    { Button::LEFT, "Left" },
    { Button::SELECT, "Select" },
    { Button::CANCEL, "Cancel" },
    { Button::CYCLE_LEFT, "Cycle L" },
    { Button::CYCLE_RIGHT, "Cycle R" },
    { Button::RUN, "Run" },
    { Button::DANCE, "Dance" },
    { Button::ATTACK, "Attack" },
    { Button::TOSS, "Toss" },
};

static Button _keycode_to_button(SDL_Keycode keycode) {
    Button button;

    if (ktobutton_map.find(keycode) == ktobutton_map.end()) {
        return Button::NULL_BUTTON;
    }

    return ktobutton_map[keycode];
}

static Button _gamepad_to_button(SDL_GamepadButton gamepad_button) {
    Button button;

    switch (gamepad_button) {
        case SDL_GAMEPAD_BUTTON_SOUTH          :button = Button::SELECT; break;
        case SDL_GAMEPAD_BUTTON_EAST           :button = Button::CANCEL; break;
        case SDL_GAMEPAD_BUTTON_WEST           :button = Button::ATTACK; break;
        case SDL_GAMEPAD_BUTTON_NORTH          :button = Button::TOSS; break;
        case SDL_GAMEPAD_BUTTON_START          :button = Button::MENU; break;
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER  :button = Button::CYCLE_LEFT; break;
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER :button = Button::CYCLE_RIGHT; break;
        case SDL_GAMEPAD_BUTTON_DPAD_UP        :button = Button::UP; break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT     :button = Button::RIGHT; break;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN      :button = Button::DOWN; break;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT      :button = Button::LEFT; break;
        default                                :button = Button::NULL_BUTTON; break;
    }

    return button;
}

void Controller::clear_hits() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        this->is_hit_map[i] = false;
    }
    this->c = NO_CHAR;
}

bool Controller::is_hit(Button button) {
    if (this->is_hit_map[button]) {
        this->is_hit_map[button] = false;
        return true;
    }

    return false;
}

bool Controller::is_down(Button button) {
    return this->is_down_map[button];
}

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
        if (this->is_down_map[Button::RUN] && keycode >= 'a' && keycode <= 'z')
            this->c = char(keycode - 32);
        else if (this->is_down_map[Button::RUN] && keycode == '-')
            this->c = '_';
        else
            this->c = char(keycode);
    }

    Button button = _keycode_to_button(keycode);
    if (button == NULL_BUTTON) {
        return;
    }

    this->is_hit_map[button] = true;
    this->is_down_map[button] = true;
}

void Controller::handle_key_up(SDL_Keycode keycode) {
    Button button = _keycode_to_button(keycode);
    if (button == NULL_BUTTON) {
        return;
    }

    this->is_down_map[button] = false;
}

void Controller::handle_gamepad_down(SDL_GamepadButton gamepad_button) {
    Button button = _gamepad_to_button(gamepad_button);
    if (button == Button::NULL_BUTTON) {
        return;
    }

    this->is_hit_map[button] = true;
    this->is_down_map[button] = true;
}

void Controller::handle_gamepad_up(SDL_GamepadButton gamepad_button) {
    Button button = _gamepad_to_button(gamepad_button);
    if (button == Button::NULL_BUTTON) {
        return;
    }

    this->is_down_map[button] = false;
}

void Controller::handle_gamepad_axis(SDL_GamepadAxis axis, Sint16 value) {
    switch (axis) {
    // "fake" axis
    case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
        if (abs(value) < 8000) {
            this->is_down_map[Button::RUN] = false;
        } else if (this->is_down_map[Button::RUN] == false) {
            this->is_hit_map[Button::RUN] = true;
            this->is_down_map[Button::RUN] = true;
        }
        return;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
        if (abs(value) < 8000) {
            this->is_down_map[Button::DANCE] = false;
        } else if (this->is_down_map[Button::DANCE] == false) {
            this->is_hit_map[Button::DANCE] = true;
            this->is_down_map[Button::DANCE] = true;
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

    float distance = (float)distance_between_points(0, 0, float(this->stick_x), float(this->stick_y));
    if (distance < 8000.0f) {
        this->is_down_map[Button::UP] = this->is_down_map[Button::RIGHT] = this->is_down_map[Button::DOWN] = this->is_down_map[Button::LEFT] = false;
        return;
    }

    int x_dir, y_dir;
    dir_to_point(0, 0, float(this->stick_x), float(this->stick_y), &x_dir, &y_dir);

    if (x_dir == 0) {
        this->is_down_map[Button::RIGHT] = this->is_down_map[Button::LEFT] = false;
    } else if (x_dir > 0) {
        this->is_down_map[Button::LEFT] = false;
        if (!this->is_down_map[Button::RIGHT]) {
            this->is_hit_map[Button::RIGHT] = true;
            this->is_down_map[Button::RIGHT] = true;
        }
    } else {
        this->is_down_map[Button::RIGHT] = false;
        if (!this->is_down_map[Button::LEFT]) {
            this->is_hit_map[Button::LEFT] = true;
            this->is_down_map[Button::LEFT] = true;
        }
    }

    if (y_dir == 0) {
        this->is_down_map[Button::DOWN] = this->is_down_map[Button::UP] = false;
    } else if (y_dir > 0) {
        this->is_down_map[Button::UP] = false;
        if (!this->is_down_map[Button::DOWN]) {
            this->is_hit_map[Button::DOWN] = true;
            this->is_down_map[Button::DOWN] = true;
        }
    } else {
        this->is_down_map[Button::DOWN] = false;
        if (!this->is_down_map[Button::UP]) {
            this->is_hit_map[Button::UP] = true;
            this->is_down_map[Button::UP] = true;
        }
    }
}
