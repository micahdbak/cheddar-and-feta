#include "controller.h"
#include "game.h"
#include "save_data.h"

#include <SDL3/SDL.h>

#include <iostream>
#include <cstdlib>

SDL_Window *window;
SDL_Renderer *renderer;
std::unordered_map<SDL_JoystickID, SDL_Gamepad *> gamepads;

bool _running;
std::unordered_map<SDL_KeyboardID, Controller *> keyboard_controllers;
std::unordered_map<SDL_JoystickID, Controller *> joystick_controllers;
Controller all_inputs("All inputs"), *player1 = nullptr, *player2 = nullptr;
Game *game;

void cleanup();
void scale_screen_rect(SDL_FRect *screen_rect, const int window_width, const int window_height);

int main(int argc, const char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int window_width = SCREEN_WIDTH*2, window_height = SCREEN_HEIGHT*2;
    if (!SDL_CreateWindowAndRenderer(argv[0], window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        std::cerr << "SDL_CreateWindowAndRenderer error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "Renderer: " << SDL_GetRendererName(renderer) << std::endl;

    SDL_SetRenderVSync(renderer, 1);

    if (std::atexit(cleanup) != 0) {
        std::cerr << "main error: couldn't register exit function" << std::endl;
        cleanup();
        return 1;
    }

    SDL_FRect screen_rect;
    scale_screen_rect(&screen_rect, window_width, window_height);

    int ret = save.load_file(0);
    if (ret == LOAD_TAMPER) {
        save.data["tampered"] = "true";
        std::cout << "tampered - but ignoring" << std::endl;
    }

    _running = true;
    game = new Game();
    game->argc = argc;
    game->argv = argv;
    game->init();
    SDL_SetWindowTitle(window, game->title.c_str());

    while (_running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_WINDOW_RESIZED:
                SDL_GetWindowSizeInPixels(window, &window_width, &window_height);
                scale_screen_rect(&screen_rect, window_width, window_height);
                break;

            // case SDL_EVENT_KEYBOARD_ADDED (unnecessary)

            case SDL_EVENT_KEYBOARD_REMOVED:
                if (keyboard_controllers.contains(event.kdevice.which)) {
                    Controller *controller = keyboard_controllers[event.kdevice.which];
                    if (player1 == controller) player1 = nullptr;
                    if (player2 == controller) player2 = nullptr;
                    keyboard_controllers.erase(event.key.which);
                    delete controller;
                }
                break;

            case SDL_EVENT_KEY_DOWN:
                // consider a keyboard "added" only when a key has been pressed
                if (!keyboard_controllers.contains(event.key.which)) {
                    const char *keyboard_name = SDL_GetKeyboardNameForID(event.key.which);
                    if (keyboard_name == NULL) break;

                    std::string name;
                    if (keyboard_name[0] == '\0') {
                        char _name[256];
                        snprintf(_name, sizeof(_name), "Keyboard %d", event.key.which);
                        name = _name;
                    } else {
                        name = keyboard_name;
                    }

                    keyboard_controllers[event.key.which] = new Controller(name);
                    if (player1 == nullptr) {
                        player1 = keyboard_controllers[event.key.which];
                    }
                }

                if (!event.key.repeat) {
                    keyboard_controllers[event.key.which]->handle_key_down(event.key.key);
                    all_inputs.handle_key_down(event.key.key);
                }
                break;

            case SDL_EVENT_KEY_UP:
                keyboard_controllers[event.key.which]->handle_key_up(event.key.key);
                all_inputs.handle_key_up(event.key.key);
                break;

            case SDL_EVENT_GAMEPAD_ADDED: {
                SDL_Gamepad *gamepad = SDL_OpenGamepad(event.gdevice.which);
                gamepads[event.gdevice.which] = gamepad;

                std::string name;
                const char *gamepad_name = SDL_GetGamepadName(gamepad);
                if (gamepad_name == NULL) {
                    char _name[256];
                    snprintf(_name, sizeof(_name), "Gamepad %d", event.gdevice.which);
                    name = _name;
                } else {
                    name = gamepad_name;
                }

                joystick_controllers[event.gdevice.which] = new Controller(name);

                if (player2 == nullptr) {
                    player2 = joystick_controllers[event.gdevice.which];
                }
            } break;

            case SDL_EVENT_GAMEPAD_REMOVED:
                if (gamepads.contains(event.gdevice.which)) {
                    SDL_CloseGamepad(gamepads[event.gdevice.which]);
                    gamepads.erase(event.gdevice.which);

                    Controller *controller = joystick_controllers[event.gdevice.which];
                    if (player1 == controller) player1 = nullptr;
                    if (player2 == controller) player2 = nullptr;
                    joystick_controllers.erase(event.gdevice.which);
                    delete controller;
                }
                break;

            case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
                Controller *controller = joystick_controllers[event.gbutton.which];
                controller->handle_gamepad_down(SDL_GamepadButton(event.gbutton.button));
                all_inputs.handle_gamepad_down(SDL_GamepadButton(event.gbutton.button));
            } break;

            case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                Controller *controller = joystick_controllers[event.gbutton.which];
                controller->handle_gamepad_up(SDL_GamepadButton(event.gbutton.button));
                all_inputs.handle_gamepad_up(SDL_GamepadButton(event.gbutton.button));
            } break;

            case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                Controller *controller = joystick_controllers[event.gbutton.which];
                controller->handle_gamepad_axis(SDL_GamepadAxis(event.gaxis.axis), event.gaxis.value);
                all_inputs.handle_gamepad_axis(SDL_GamepadAxis(event.gaxis.axis), event.gaxis.value);
            } break;

            case SDL_EVENT_QUIT:
                _running = false;
                break;

            default:
                // pass
                break;
            }
        }

        game->step();

        for (auto pair : keyboard_controllers)
            pair.second->clear_hits();
        for (auto pair : joystick_controllers)
            pair.second->clear_hits();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, game->screen, NULL, &screen_rect);
        SDL_RenderPresent(renderer);
    }

    exit(0);
}

void cleanup() {
    delete game;
    game = nullptr;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void scale_screen_rect(SDL_FRect *screen_rect, const int window_width, const int window_height) {
    static const float screen_aspect_ratio_wtoh = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
    static const float screen_aspect_ratio_htow = float(SCREEN_HEIGHT) / float(SCREEN_WIDTH);

    float window_aspect_ratio = float(window_width) / float(window_height);

    if (window_aspect_ratio > screen_aspect_ratio_wtoh) {
        screen_rect->w = screen_aspect_ratio_wtoh * float(window_height);
        screen_rect->h = float(window_height);
        screen_rect->x = 0.5f * (float(window_width) - screen_rect->w);
        screen_rect->y = 0.0f;
    } else {
        screen_rect->w = float(window_width);
        screen_rect->h = screen_aspect_ratio_htow * float(window_width);
        screen_rect->x = 0.0f;
        screen_rect->y = 0.5f * (float(window_height) - screen_rect->h);
    }
}
