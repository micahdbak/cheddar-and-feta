#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <unordered_map>
#include <queue>

#include <SDL3/SDL.h>

#define NO_CHAR -1

class Keyboard {
public:
    Keyboard();
    ~Keyboard();

    void clear_hits();
    bool is_hit(SDL_Keycode key);
    bool is_down(SDL_Keycode key);

    void handle_down(SDL_Keycode key);
    void handle_up(SDL_Keycode key);

    char c = NO_CHAR;

private:
    std::unordered_map<SDL_Keycode, bool> is_hit_map;
    std::unordered_map<SDL_Keycode, bool> is_down_map;
};

extern Keyboard keyboard;

#endif
