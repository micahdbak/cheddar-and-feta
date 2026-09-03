#include "save_data.h"

#include <sys/stat.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "game.h"

#ifdef _WIN32
#include <direct.h>  // _mkdir
#endif

SaveData save;

static void _mkdir_if_not_exists(char* save_root, size_t save_root_size) {
// macOS or Linux/*BSD/UNIX systems
#if defined(__APPLE__) || defined(__unix__)
  const char* home = getenv("HOME");
  if (home == NULL) {
    std::cerr << "SaveData::_mkdir_if_not_exists error: $HOME unset."
              << std::endl;
    exit(1);
  }
  snprintf(save_root, save_root_size,
           "%s/Library/Application Support/CheddarAndFeta", home);

  if (mkdir(save_root, 0755) == 0) return;
// windows
#elif defined(_WIN32)
  // use getenv("AppData") to get that dir
  const char* app_data = getenv("AppData");
  if (app_data == NULL) {
    std::cerr << "SaveData::_mkdir_if_not_exists error: %%AppData%% unset."
              << std::endl;
    exit(1);
  }
  snprintf(save_root, save_root_size, "%s/CheddarAndFeta", app_data);

  if (_mkdir(save_root) == 0) return;
#endif

  // if directory already exists, that's okay
  if (errno != EEXIST) {
    std::cerr << "SaveData::_mkdir_if_not_exists error: mkdir has errno "
              << errno << "." << std::endl;
    exit(1);
  }
}

#define CORRUPTED_EXIT                                                         \
  {                                                                            \
    std::cerr << "save_data.cpp (" << __LINE__ << "): bad save." << std::endl; \
    exit(1);                                                                   \
  }

std::vector<std::string> SaveData::file_summaries() {
  char save_root[1024], save_file_name[1024];
  _mkdir_if_not_exists(save_root, sizeof(save_root));
  std::vector<std::string> summaries;

  for (int i = 0;; i++) {
    // open save file
    snprintf(save_file_name, sizeof(save_file_name), "%s/save%d.txt", save_root,
             i);
    FILE* save_file = fopen(save_file_name, "r");
    if (save_file == NULL) {
      break;
    }

    // read first line of file
    char line[1024];
    if (fgets(line, sizeof(line), save_file) == NULL) break;

    line[strcspn(line, "\n")] = '\0';

    // first line of file is save summary
    summaries.push_back(line);
    fclose(save_file);
  }

  return summaries;
}

// http://www.cse.yorku.ca/~oz/hash.html
static unsigned long _djb2_hash(unsigned long starting_hash, const char* str) {
  unsigned long hash = starting_hash == 0 ? 5381 : starting_hash;
  int c;

  while ((c = *str++) != '\0')
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

void SaveData::write_file(int file_i) {
  if (file_i < 0) CORRUPTED_EXIT

  char save_root[1024], save_file_name[1024];
  _mkdir_if_not_exists(save_root, sizeof(save_root));

  // open file
  snprintf(save_file_name, sizeof(save_file_name), "%s/save%d.txt", save_root,
           file_i);
  FILE* save_file = fopen(save_file_name, "w");
  if (save_file == NULL) CORRUPTED_EXIT

  std::string line;
  unsigned long hash = 0;

  if (this->geti(GAME_DONE)) {
    line = "(Game Complete.)\n";
  } else {
    line = game->map_title + ", " + game->map_description + "\n";
  }

  hash = _djb2_hash(hash, line.c_str());
  fputs(line.c_str(), save_file);

  // write rest of save data to file
  for (auto pair : this->data) {
    if (pair.first[0] == DONT_WRITE[0]) continue;

    if (pair.first.empty() || pair.second.empty()) continue;

    line = pair.first + "=" + pair.second + "\n";
    hash = _djb2_hash(hash, line.c_str());
    fputs(line.c_str(), save_file);
  }

  // output hash of file to prevent tampering (not super secure, just enough)
  char buff[256];
  snprintf(buff, sizeof(buff), "%lu\n", hash);
  fputs(buff, save_file);

  fclose(save_file);
}

#define MAX_LINE_LENGTH 10240  // 10kb

int SaveData::load_file(int file_i) {
  if (file_i < 0) CORRUPTED_EXIT

  char save_root[1024], save_file_name[1024];
  _mkdir_if_not_exists(save_root, sizeof(save_root));

  this->data.clear();

  // open file
  snprintf(save_file_name, sizeof(save_file_name), "%s/save%d.txt", save_root,
           file_i);
  FILE* save_file = fopen(save_file_name, "r");
  if (save_file == NULL) {
    return LOAD_NEW;
  }

  char line[MAX_LINE_LENGTH];
  unsigned long hash = 0;

  fgets(line, sizeof(line), save_file);
  hash = _djb2_hash(hash, line);

  // for every subsequent line in the file
  while (fgets(line, sizeof(line), save_file) != NULL) {
    // remove last newline
    size_t len = strnlen(line, MAX_LINE_LENGTH);

    if (len == 0) continue;

    char *key = line, *val;
    bool is_kvp = false;

    for (char* ptr = line; *ptr != '\0'; ptr++) {
      if (*ptr == '=') {
        // hash this line; must include the = and \n chars
        hash = _djb2_hash(hash, line);
        *ptr = '\0';
        val = ptr + 1;
        is_kvp = true;
      }
    }

    // remove newline from line so val is just the chars between = and \n
    if (line[len - 1] == '\n') line[len - 1] = '\0';

    if (!is_kvp && line[0] >= '0' && line[0] <= '9') {
      // last line of file - hash
      unsigned long hash_in_file;
      if (1 != sscanf(line, "%lu\n", &hash_in_file)) FATAL_ERROR
      fclose(save_file);
      return hash == hash_in_file ? LOAD_SUCCESS : LOAD_TAMPER;
    }

    this->data[key] = val;
  }

  fclose(save_file);

  // there wasn't a hash...?
  return LOAD_TAMPER;
}

void float_to_str(float f, char* str, size_t str_size) {
  if (str_size < 9) return;
  uint32_t bits;
  memcpy(&bits, &f, 4);
  snprintf(str, str_size, "%x", bits);
}

float str_to_float(std::string str) {
  const char* arr = str.c_str();
  uint32_t bits = strtoul(arr, NULL, 16);
  float f;
  memcpy(&f, &bits, 4);
  return f;
}
