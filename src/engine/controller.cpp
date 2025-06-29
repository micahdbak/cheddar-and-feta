#include "SDL3/SDL_keycode.h"
#include "game.h"
#include "controller.h"

#include <iostream>

static Button _keycode_to_button(SDL_Keycode keycode) {
    Button button;

    switch (keycode) {
        case SDLK_ESCAPE    :button = PAUSE; break;
        case SDLK_TAB       :button = MENU; break;
        case SDLK_LSHIFT    :button = L2; break;
        case SDLK_RSHIFT    :button = R2; break;
        case SDLK_LEFT      :button = L1; break;
        case SDLK_RIGHT     :button = R1; break;
        case SDLK_RETURN    :button = PRIMARY; break;
        case SDLK_BACKSPACE :button = SECONDARY; break;
        case SDLK_SPACE     :button = ACTION1; break;
        case SDLK_C         :button = ACTION2; break;
        case SDLK_W         :button = UP; break;
        case SDLK_D         :button = RIGHT; break;
        case SDLK_S         :button = DOWN; break;
        case SDLK_A         :button = LEFT; break;
        default             :button = NULL_BUTTON; break;
    }

    return button;
}

static Button _gamepad_to_button(SDL_GamepadButton gamepad_button) {
    Button button;

    switch (gamepad_button) {
        case SDL_GAMEPAD_BUTTON_SOUTH          :button = PRIMARY; break;
        case SDL_GAMEPAD_BUTTON_EAST           :button = SECONDARY; break;
        case SDL_GAMEPAD_BUTTON_WEST           :button = ACTION1; break;
        case SDL_GAMEPAD_BUTTON_NORTH          :button = ACTION2; break;
        case SDL_GAMEPAD_BUTTON_BACK           :button = PAUSE; break;
        case SDL_GAMEPAD_BUTTON_START          :button = MENU; break;
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER  :button = L1; break;
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER :button = R1; break;
        case SDL_GAMEPAD_BUTTON_DPAD_UP        :button = UP; break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT     :button = RIGHT; break;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN      :button = DOWN; break;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT      :button = LEFT; break;
        default                                :button = NULL_BUTTON; break;
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
        if (this->is_down_map[L2] && keycode >= 'a' && keycode <= 'z')
            this->c = char(keycode - 32);
        else if (this->is_down_map[L2] && keycode == '-')
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
    if (button == NULL_BUTTON) {
        return;
    }

    this->is_hit_map[button] = true;
    this->is_down_map[button] = true;
}

void Controller::handle_gamepad_up(SDL_GamepadButton gamepad_button) {
    Button button = _gamepad_to_button(gamepad_button);
    if (button == NULL_BUTTON) {
        return;
    }

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
            this->is_down_map[L2] = true;
        }
        return;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
        if (abs(value) < 8000) {
            this->is_down_map[R2] = false;
        } else if (this->is_down_map[R2] == false) {
            this->is_hit_map[R2] = true;
            this->is_down_map[R2] = true;
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
