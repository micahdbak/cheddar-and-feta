#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>

#include <SDL3/SDL.h>

#define NO_CHAR -1

enum Button {
    PAUSE = 0,
    MENU,
    L2,
    R2,
    L1,
    R1,
    PRIMARY,
    SECONDARY,
    ACTION1,
    ACTION2,
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NUM_BUTTONS,
    NULL_BUTTON
};

class Controller {
public:
    Controller() = default;
    ~Controller() = default;

    void clear_hits();
    bool is_hit(Button button);
    bool is_down(Button button);

    // network-connected player
    void handle_button_hit(Button button);
    void handle_button_down(Button button);
    void handle_button_up(Button button);

    // keyboard
    void handle_key_down(SDL_Keycode keycode);
    void handle_key_up(SDL_Keycode keycode);

    // controller
    void handle_gamepad_down(SDL_GamepadButton gamepad_button);
    void handle_gamepad_up(SDL_GamepadButton gamepad_button);
    void handle_gamepad_axis(SDL_GamepadAxis gamepad_axis, Sint16 value);

    char c = NO_CHAR;

    bool is_hit_map[NUM_BUTTONS] = {0};
    bool is_down_map[NUM_BUTTONS] = {0};

    Sint16 stick_x, stick_y;
};

extern Controller local_controller, remote_controller;

#endif
