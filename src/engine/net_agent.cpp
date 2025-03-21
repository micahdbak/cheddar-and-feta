#include "net_agent.h"

#include <rtc/rtc.h>

#include <chrono>
#include <iostream>
#include <random>

NetworkAgent *net_agent = nullptr;

NetworkAgent::NetworkAgent(bool offerer) {
    if (net_agent != nullptr) {
        std::cerr << "NetworkAgent::NetworkAgent: a network agent already exists." << std::endl;
        std::exit(1);
    }

    net_agent = this;
    this->offerer = offerer;
    rtcPreload();

    // start message loop
    this->message_thread = new std::thread(NetworkAgent::message_loop);
}

NetworkAgent::~NetworkAgent() {
    {
        std::lock_guard<std::mutex> guard(this->stop_message_loop_mutex);
        this->stop_message_loop = true;
    }
    this->message_thread->join();
    this->message_thread = nullptr;
    rtcCleanup();
}

static std::string make_code() {
    static char charset[] = "ABCEFHJKLMNPQRTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset)-2);
    
    std::string ret;
    for (int i = 0; i < 6; i++) {
        char c = charset[dis(gen)];
        ret.push_back(c);
    }

    return ret;
}

void NetworkAgent::no_connection() {
    std::string connection_code;

    // get connection code either from user or generate
    if (net_agent->offerer) {
        connection_code = net_agent->get_connection_code();

        // wait 500ms and check again
        if (connection_code.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            return;
        }
    } else {
        connection_code = make_code();
    }

    // attempt to connect using connection code
    std::string url = SIGNALLING_URL + connection_code;
    net_agent->ws = rtcCreateWebSocket(url.c_str());

    if (net_agent->ws >= 0) {
        // websocket server should send a code
        if (!net_agent->offerer) {
            net_agent->set_connection_code(connection_code);
        }

        net_agent->set_state(NetworkAgent::State::WAITING_FOR_PEER);
    } else {
        // couldn't connect, try again in 500ms and hope the network conditions changed
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void NetworkAgent::waiting_for_peer() {
    int size = 0;
    int rtc_err = rtcReceiveMessage(net_agent->ws, NULL, &size);

    if (rtc_err == RTC_ERR_SUCCESS && size < 0) {
        char *buffer = new char[size * -1];
        rtcReceiveMessage(net_agent->ws, buffer, &size);

        if (buffer[0] == 'p') {
            rtcSendMessage(net_agent->ws, "ping", -1);
            std::cout << "ping back!" << std::endl;
        } else {
            std::cout << buffer << std::endl;
        }
    } else if (rtc_err == RTC_ERR_NOT_AVAIL) {
        // no messages available; wait a bit
        rtcSendMessage(net_agent->ws, "ping", -1);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    } else {
        // an unexpected error occurred; delete the websocket and retry
        rtcDelete(net_agent->ws);
        net_agent->ws = -1;
        net_agent->set_state(NetworkAgent::State::NO_CONNECTION);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void NetworkAgent::message_loop() {
    int ws = -1;

    while (!net_agent->should_stop()) {
        switch (net_agent->get_state()) {
        case NetworkAgent::State::NO_CONNECTION:
            NetworkAgent::no_connection();
            break;

        case NetworkAgent::State::WAITING_FOR_PEER:
            NetworkAgent::waiting_for_peer();
            break;

        default: break;
        }
    }

    if (net_agent->ws != -1) {
        rtcDelete(net_agent->ws);
        net_agent->ws = -1;
    }
}
