#include "game.h"

// objects
#include "mouse.h"

void Game::init() {
    this->factories[MOUSE_OBJ] = new MouseFactory();
    this->load_map("maps/house.map");

    this->title = "Cheddar n' Feta";
}
