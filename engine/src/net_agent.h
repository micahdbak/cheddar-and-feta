#ifndef NET_AGENT_H
#define NET_AGENT_H

#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "net_connection.h"
#include "net_signaller.h"

// shared message codes between cheddar and feta

// feta -> cheddar
#define MSG_USE_ITEM '0'
#define MSG_TOSS_ITEM '1'
#define MSG_PUSH_HITBOX '2'
#define MSG_FETA_INFO '3'
#define MSG_EAT_CHEESE '4'
#define MSG_IS_DOWN '5'

// cheddar -> feta
#define MSG_MAP 'a'
#define MSG_SPRITE 'b'
#define MSG_AUDIO 'c'
#define MSG_SYNC 'd'
#define MSG_ATTACK 'e'
#define MSG_PUSH_ITEM 'f'
#define MSG_PUSH_CHEESE 'g'
#define MSG_FORCE_DANCE 'h'
#define MSG_DID_HIT 'i'
#define MSG_THROW 'j'
#define MSG_MAX_SPEED 'k'
#define MSG_COLLISION 'l'

class NetworkAgent {
 public:
  enum State { NO_CONNECTION, WAITING_FOR_PEER, CONNECTED, TRY_RESET };

  NetworkAgent(bool offerer);
  ~NetworkAgent();

  State get_state() {
    std::lock_guard<std::mutex> guard(this->state_mutex);
    return this->state;
  }

  void try_reset() {
    std::lock_guard<std::mutex> guard(this->state_mutex);
    this->state = NetworkAgent::State::TRY_RESET;
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
    while (!this->received_messages.empty()) this->received_messages.pop();
    return message;
  }

  std::queue<std::string> all_messages() {
    std::lock_guard<std::mutex> guard(this->received_messages_mutex);
    if (this->received_messages.empty()) {
      return std::queue<std::string>();
    }

    std::queue<std::string> cpy(this->received_messages);
    // clear
    while (!this->received_messages.empty()) this->received_messages.pop();
    return cpy;
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

  static void no_connection();
  static void waiting_for_peer();
  static void connected();
  static void message_loop();

  bool offerer;
  std::thread* message_thread;

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

  // only to be used in message_thread
  NetworkConnection connection;
  NetworkSignaller signaller;
  int pc = -1, dc = -1;
  bool got_ping = false;
};

extern NetworkAgent* net_agent;

#endif
