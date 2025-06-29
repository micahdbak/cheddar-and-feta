#include "save_data.h"

#include <cerrno>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h> // _mkdir
#endif

SaveData save;

static void _mkdir_if_not_exists(char *save_root, size_t save_root_size) {
// macOS or Linux/*BSD/UNIX systems
#if defined(__APPLE__) || defined(__unix__)
    const char *home = getenv("HOME");
    if (home == NULL) {
        std::cerr << "SaveData::_mkdir_if_not_exists error: $HOME unset." << std::endl;
        exit(1);
    }
    snprintf(save_root, save_root_size, "%s/Library/Application Support/CheddarAndFeta", home);

    if (mkdir(save_root, 0755) == 0)
        return;
// windows
#elif defined(_WIN32)
    // use getenv("AppData") to get that dir
    const char *app_data = getenv("AppData");
    if (app_data == NULL) {
        std::cerr << "SaveData::_mkdir_if_not_exists error: %%AppData%% unset." << std::endl;
        exit(1);
    }
    snprintf(save_root, save_root_size, "%s/CheddarAndFeta", app_data);

    if (_mkdir(save_root) == 0)
        return;
#endif

    // if directory already exists, that's okay
    if (errno != EEXIST) {
        std::cerr << "SaveData::_mkdir_if_not_exists error: mkdir has errno " << errno << "." << std::endl;
        exit(1);
    }
}

#define CORRUPTED_EXIT \
{\
    std::cerr << "save_data.cpp (" << __LINE__ << "): bad save." << std::endl;\
    exit(1);\
}

#define MAX_LINE_LENGTH 1024

std::vector<std::string> SaveData::file_summaries() {
    char save_root[1024], save_file_name[1024];
    _mkdir_if_not_exists(save_root, sizeof(save_root));
    std::vector<std::string> summaries;

    for (int i = 0; i < NUM_SAVE_FILES; i++) {
        // open save file
        snprintf(save_file_name, sizeof(save_file_name), "%s/save%d.txt", save_root, i);
        FILE *save_file = fopen(save_file_name, "r");
        if (save_file == NULL) {
            // if file doesn't exist, push back "Empty." summary
            summaries.push_back("Empty.");
            continue;
        }

        // read first line of file
        char line[MAX_LINE_LENGTH];
        if (fgets(line, sizeof(line), save_file) == NULL) CORRUPTED_EXIT
        line[strcspn(line, "\n")] = '\0';

        // first line of file is save summary
        summaries.push_back(line);
        fclose(save_file);
    }

    return summaries;
}

// http://www.cse.yorku.ca/~oz/hash.html
static unsigned long _djb2_hash(unsigned long starting_hash, char *str) {
    unsigned long hash = starting_hash == 0 ? 5381 : starting_hash;
    int c;

    while ((c = *str++) != '\0')
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void SaveData::write_file(int file_i) {
    if (file_i < 0 || file_i >= NUM_SAVE_FILES) CORRUPTED_EXIT

    char save_root[1024], save_file_name[1024];
    _mkdir_if_not_exists(save_root, sizeof(save_root));

    // open file
    snprintf(save_file_name, sizeof(save_file_name), "%s/save%d.txt", save_root, file_i);
    FILE *save_file = fopen(save_file_name, "w");
    if (save_file == NULL) CORRUPTED_EXIT

    if (!this->data.contains("level"))
        this->data["level"] = "0";

    char line[MAX_LINE_LENGTH];
    unsigned long hash = 0;

    // first line is summary
    snprintf(line, sizeof(line), "Level %d\n", atoi(this->data["level"].c_str()));
    hash = _djb2_hash(hash, line);
    fputs(line, save_file);

    // write rest of save data to file
    for (auto pair : this->data) {
        snprintf(line, sizeof(line), "%s=%s\n", pair.first.c_str(), pair.second.c_str());
        hash = _djb2_hash(hash, line);
        fputs(line, save_file);
    }

    // output hash of file to prevent tampering (not super secure, just enough)
    snprintf(line, sizeof(line), "%lu\n", hash);
    fputs(line, save_file);

    fclose(save_file);
}

int SaveData::load_file(int file_i) {
    if (file_i < 0 || file_i >= NUM_SAVE_FILES) CORRUPTED_EXIT

    char save_root[1024], save_file_name[1024];
    _mkdir_if_not_exists(save_root, sizeof(save_root));

    this->data.clear();

    // open file
    snprintf(save_file_name, sizeof(save_file_name), "%s/save%d.txt", save_root, file_i);
    FILE *save_file = fopen(save_file_name, "r");
    if (save_file == NULL) {
        return LOAD_NEW;
    }

    char line[MAX_LINE_LENGTH];
    unsigned long hash = 0;

    fgets(line, sizeof(line), save_file);
    hash = _djb2_hash(hash, line);

    // for every subsequent line in the file
    while (fgets(line, sizeof(line), save_file) != NULL) {
        char key[1024], val[1024];
        int nread = sscanf(line, "%[^=]=%[^\n]", key, val);

        // last line of file - hash
        if (nread < 2) {
            unsigned long hash_in_file;
            nread = sscanf(line, "%lu\n", &hash_in_file);
            fclose(save_file);

            return hash != hash_in_file ? LOAD_TAMPER : LOAD_SUCCESS;
        } else {
            hash = _djb2_hash(hash, line);
        }

        this->data[key] = val;
    }

    fclose(save_file);

    // there wasn't a hash...?
    return LOAD_TAMPER;
}

void float_to_str(float f, char *str, size_t str_size) {
    if (str_size < 9) return;
    uint32_t bits;
    memcpy(&bits, &f, 4);
    snprintf(str, str_size, "%x", bits);
}

float str_to_float(const char *str) {
    uint32_t bits = strtoul(str, NULL, 16);
    float f;
    memcpy(&f, &bits, 4);
    return f;
}
