#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include <map>
#include <string>
#include <vector>

#define NUM_SAVE_FILES 3

#define LOAD_SUCCESS 0
#define LOAD_NEW     1
#define LOAD_TAMPER  2

class SaveData {
public:
    SaveData() = default;
    ~SaveData() = default;

    std::vector<std::string> file_summaries();
    void write_file(int file_i);
    int load_file(int file_i);

    std::unordered_map<std::string, std::string> data;
};

void float_to_str(float f, char *str, size_t str_size);
float str_to_float(const char *str);

extern SaveData save;

#endif
