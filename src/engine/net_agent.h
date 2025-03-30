#ifndef NET_AGENT_H
#define NET_AGENT_H

#include <mutex>
#include <queue>
#include <string>
#include <thread>

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
        if (this->received_messages.empty()) {
            return "";
        }

        std::string message = this->received_messages.front();
        this->received_messages.pop();
        return message;
    }

    std::string last_message() {
        std::lock_guard<std::mutex> guard(this->received_messages_mutex);
        if (this->received_messages.empty()) {
            return "";
        }

        std::string message = this->received_messages.back();
        // clear
        while (!this->received_messages.empty())
            this->received_messages.pop();
        return message;
    }

    void send_message(std::string message) {
        std::lock_guard<std::mutex> guard(this->send_messages_mutex);

        // max 10 messages in queue
        if (this->send_messages.size() >= 10) {
            // drop the oldest message and prefer more recent messages
            this->send_messages.pop();
        }

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

    void push_message(std::string message) {
        std::lock_guard<std::mutex> guard(this->received_messages_mutex);
        this->received_messages.push(message);
    }

    static void initiate();
    static void reset(bool delete_ws);

    static bool next_ws_message(std::string &str);

    static void description_cb(int, const char *sdp, const char *type, void *);
    static void candidate_cb(int, const char *cand, const char *mid, void *);
    static void datachannel_cb(int, int dc, void *);
    static void message_cb(int, const char *message, int size, void *);
    static void error_cb(int id, const char *error, void *);
    static void open_cb(int id, void *);
    static void close_cb(int id, void *);

    static void no_connection();
    static void waiting_for_peer();
    static void connected();
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
    //std::condition_variable cv;

    int ws = -1, pc = -1, dc = -1;
    bool initiated = false;
};

extern NetworkAgent *net_agent;

#endif
