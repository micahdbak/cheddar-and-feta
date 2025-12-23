#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <map>

#include <SDL3/SDL.h>

#define NO_CHAR -1

enum Button {
    MENU = 0,
    UP,
    RIGHT,
    DOWN,
    LEFT,
    SELECT,
    CANCEL,
    TOSS,
    ATTACK,
    DANCE,
    DIGIT,
    NUM_BUTTONS,
    NULL_BUTTON
};

extern std::map<SDL_Keycode, Button> ktobutton_map;
extern std::map<Button, SDL_Keycode> btokeycode_map;
extern std::map<Button, std::string> btostring_map;

class Controller {
public:
    Controller() = default;
    ~Controller() = default;

    void clear_hits();
    void clear_all();
    bool is_hit(Button button);
    bool is_down(Button button);

    // network-connected player
    void handle_button_hit(Button button);
    void handle_button_down(Button button);
    void handle_button_up(Button button);

    // keyboard
    void handle_key_down(SDL_Keycode keycode);
    void handle_key_up(SDL_Keycode keycode);

    char c = NO_CHAR;
    int digit = 0;

    bool is_hit_map[NUM_BUTTONS] = {0};
    bool is_down_map[NUM_BUTTONS] = {0};

    Sint16 stick_x, stick_y;

    SDL_Keycode last_input;
};

extern Controller local_controller, remote_controller, captured_controller;
extern bool capture_controls;

#endif
