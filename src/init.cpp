#include "game.h"

// objects
#include "mouse.h"
#include "save_station.h"

void Game::init() {
    this->factories[MOUSE_OBJ] = new MouseFactory();
    this->factories[SAVE_STATION_OBJ] = new SaveStationFactory();
    this->load_map("maps/house.map");

    this->title = "Cheddar n' Feta";
}
