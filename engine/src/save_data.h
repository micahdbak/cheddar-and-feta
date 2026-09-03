#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include <string>
#include <unordered_map>
#include <vector>

#define LOAD_SUCCESS 0
#define LOAD_NEW 1
#define LOAD_TAMPER 2

#define DONT_WRITE "!"
#define LOAD_SAVE DONT_WRITE "load_save"
#define LOAD_MAP "map"
#define SAVE_FILE DONT_WRITE "save_file"
#define GAME_DONE "game_done"

#define STATS "_stats"

#define NEW_SAVE_STR "<New Save>"

void float_to_str(float f, char* str, size_t str_size);
float str_to_float(std::string str);

class SaveData {
 public:
  SaveData() = default;
  ~SaveData() = default;

  std::vector<std::string> file_summaries();
  void write_file(int file_i);
  int load_file(int file_i);

  // call this if you intend on loading a save after playing a game
  void clear() { this->data.clear(); }

  std::string value(std::string key) {
    if (this->data.find(key) == this->data.end()) {
      return "";
    }

    return this->data[key];
  }

  bool has(std::string key) { return this->data.find(key) != this->data.end(); }

  // put float
  void putf(std::string key, float val) {
    char buff[256];
    float_to_str(val, buff, sizeof(buff));
    this->data[key] = std::string(buff);
  }

  // put integer
  void puti(std::string key, int val) {
    char buff[256];
    snprintf(buff, sizeof(buff), "%d", val);
    this->data[key] = std::string(buff);
  }

  // get float (0.0f by default)
  float getf(std::string key) {
    std::string val = this->value(key);
    return val.empty() ? 0.0f : str_to_float(val);
  }

  // get integer (0 by default)
  int geti(std::string key) {
    std::string val = this->value(key);
    return val.empty() ? 0 : std::stoi(val);
  }

  std::unordered_map<std::string, std::string> data;
};

extern SaveData save;

#endif
