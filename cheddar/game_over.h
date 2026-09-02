#ifndef GAME_OVER_OBJ
#define GAME_OVER_OBJ "game_over"

#include "object.h"
#include "game.h"

class GameOver : public Object {
public:
    GameOver();
    ~GameOver();

    void step() override;

private:
    SDL_Texture *ui;
    SDL_FRect src, dst;
};

class GameOverFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        return new GameOver();
    }
};

#endif