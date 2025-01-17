#include "game.h"

#include "mouse.h"

void Game::init() {
    this->factories[MOUSE_OBJ] = new MouseFactory();
    this->load_map("maps/mouse.map");

    this->title = "Cheddar n' Feta";
}
