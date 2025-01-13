#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <SDL3/SDL.h>

#include "sprite.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

class Game {
public:
    Game();
    ~Game();

    void step();

    SDL_Texture *screen;

private:
    float delta;
    Uint64 last_ticks;

    Sprite *mouse;
};

extern SDL_Renderer *renderer;
extern Game *game;

#endif
