#include "game.h"
#include "net_agent.h"
#include "net_receiver.h"
#include "net_sender.h"

#include <iostream>

void Game::init() {
    if (game->argc < 2) {
        std::cerr << "Insufficient arguments provided." << std::endl;
        std::exit(1);
    }

    net_agent = new NetworkAgent(true);
    net_agent->set_connection_code(game->argv[1]);

    this->factories[FIRST_OBJ] = new NetReceiverFactory();
    this->create_object(FIRST_OBJ, "");
    this->factories[LAST_OBJ] = new NetSenderFactory();
    this->create_object(LAST_OBJ, "");
    game->create_objects = false;
}
