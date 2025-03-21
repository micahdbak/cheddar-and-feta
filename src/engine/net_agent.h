#ifndef NET_AGENT_H
#define NET_AGENT_H

#include <mutex>
#include <queue>
#include <string>
#include <thread>

#define SIGNALLING_URL "ws://localhost:8080/sc/"

class NetworkAgent {
public:
    enum State { NO_CONNECTION, WAITING_FOR_PEER, CONNECTED };

    NetworkAgent(bool offerer);
    ~NetworkAgent();

    State get_state() {
        std::lock_guard<std::mutex> guard(this->state_mutex);
        return this->state;
    }

    std::string get_connection_code() {
        std::lock_guard<std::mutex> guard(this->connection_code_mutex);
        return this->connection_code;
    }

    void set_connection_code(std::string code) {
        std::lock_guard<std::mutex> guard(this->connection_code_mutex);
        this->connection_code = code;
    }

    std::string next_message() {
        std::lock_guard<std::mutex> guard(this->received_messages_mutex);
        std::string message = this->received_messages.front();
        this->received_messages.pop();
        return message;
    }

    void send_message(std::string message) {
        std::lock_guard<std::mutex> guard(this->send_messages_mutex);
        this->send_messages.push(message);
    }

protected:
    bool should_stop() {
        std::lock_guard<std::mutex> guard(this->stop_message_loop_mutex);
        return this->stop_message_loop;
    }

    void set_state(State state) {
        std::lock_guard<std::mutex> guard(this->state_mutex);
        this->state = state;
    }

    static void no_connection();
    static void waiting_for_peer();
    // static void connected();
    static void message_loop();

    bool offerer;
    std::thread *message_thread;

    bool stop_message_loop = false;
    std::mutex stop_message_loop_mutex;

    State state = NO_CONNECTION;
    std::mutex state_mutex;

    std::string connection_code;
    std::mutex connection_code_mutex;

    std::queue<std::string> received_messages;
    std::mutex received_messages_mutex;

    std::queue<std::string> send_messages;
    std::mutex send_messages_mutex;

    int ws = -1, pc = -1;
};

extern NetworkAgent *net_agent;

#endif
