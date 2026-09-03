#ifndef NET_SIGNALLER_H
#define NET_SIGNALLER_H

#include <queue>
#include <string>

class NetworkSignaller {
 public:
  enum State { NOT_CONNECTED = 0, CONNECTING, CONNECTED };

  NetworkSignaller() = default;
  ~NetworkSignaller() = default;

  void connect(const std::string& code, bool offerer);
  void disconnect();

  void send(const std::string& message);
  std::string receive();
  std::string block_receive();

  State state = State::NOT_CONNECTED;

 private:
  static void on_open(int, void* user_ptr);
  static void on_close(int, void* user_ptr);
  static void on_error(int, const char* error, void* user_ptr);
  static void on_message(int, const char* message, int size, void* user_ptr);

  int ws = -1;
  std::queue<std::string> messages;
};

#endif
