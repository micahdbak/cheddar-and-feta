#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <map>

#include <SDL3/SDL.h>

class Keyboard {
public:
    Keyboard();
    ~Keyboard();

    void clear_hits();
    bool is_hit(SDL_Keycode key);
    bool is_down(SDL_Keycode key);
    void handle_down(SDL_Keycode key);
    void handle_up(SDL_Keycode key);

private:
    std::map<SDL_Keycode, bool> is_hit_map;
    std::map<SDL_Keycode, bool> is_down_map;
};

extern Keyboard keyboard;

#endif
