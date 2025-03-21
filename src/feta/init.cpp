#include "game.h"
#include "net_agent.h"
#include "object.h"

#include <iostream>

#define NET_HANDLER_OBJ "net_handler"

class NetHandler : public Object {
public:
    NetHandler() {}
    ~NetHandler() {}

    void step() override {
        NetworkAgent::State state = net_agent->get_state();
        std::string conn_str = net_agent->get_connection_code();
        switch (state) {
        case NetworkAgent::State::NO_CONNECTION:
            std::cout << conn_str << ": no connection yet." << std::endl;
            break;
        case NetworkAgent::State::WAITING_FOR_PEER:
            std::cout << conn_str << ": waiting for peer." << std::endl;
            break;
        }
    }
};

class NetHandlerFactory : public ObjectFactory {
public:
    NetHandlerFactory() {}
    ~NetHandlerFactory() {}

    Object *create(const std::string &options) {
        return new NetHandler();
    }
};

void Game::init() {
    if (game->argc < 2) {
        std::cerr << "Insufficient arguments provided." << std::endl;
        std::exit(1);
    }

    bool offerer = game->argv[1][0] - '0'; // 0 for false, 1 for true
    std::cout << (offerer ? "true" : "false") << std::endl;
    net_agent = new NetworkAgent(offerer);

    if (offerer) {
        if (game->argc < 3) {
            std::cerr << "Insufficient arguments provided." << std::endl;
            std::exit(1);
        }
        net_agent->set_connection_code(game->argv[2]);
    }

    game->factories[NET_HANDLER_OBJ] = new NetHandlerFactory();
    game->create_object(NET_HANDLER_OBJ, "");
}
