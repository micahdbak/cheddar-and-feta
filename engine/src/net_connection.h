#ifndef NET_CONNECTION
#define NET_CONNECTION

#include <queue>
#include <string>

class NetworkConnection {
 public:
  enum State { DEAD = 0, ERROR, COLLECTING, ALIVE };

  NetworkConnection() = default;
  ~NetworkConnection() = default;

  void start_collecting(bool offerer);
  void kill();

  void set_remote_description(const std::string& sdp, const std::string& type);
  std::pair<std::string, std::string> get_local_description();

  void add_candidate(const std::string& cand, const std::string& mid);
  std::pair<std::string, std::string> get_candidate();

  void send(const std::string& message);
  std::string receive();

  State state = State::DEAD;

 private:
  static void on_description(int, const char* sdp, const char* type,
                             void* user_ptr);
  static void on_candidate(int, const char* cand, const char* mid,
                           void* user_ptr);
  static void on_datachannel(int, int dc, void* user_ptr);
  static void on_open(int, void* user_ptr);
  static void on_close(int, void* user_ptr);
  static void on_error(int, const char* error, void* user_ptr);
  static void on_message(int, const char* message, int size, void* user_ptr);

  int pc = -1, dc = -1;
  std::pair<std::string, std::string> description;
  std::queue<std::pair<std::string, std::string>> candidates;
  std::queue<std::string> messages;
};

#endif
