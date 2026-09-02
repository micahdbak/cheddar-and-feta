#ifndef SPAWNER_OBJ
#define SPAWNER_OBJ "spawner"

#include "object.h"
#include "sprite.h"

#include <vector>

class FoeSpawner: public Object {
public:
    FoeSpawner(float x, float y, int spawner_id, int animation, const std::vector<std::vector<std::string>> &waves);
    ~FoeSpawner();

    void step() override;

    void trigger();

    void log_death();

    bool empty = false;
private:
    float x, y;
    int spawner_id;
    std::vector<std::vector<std::string>> waves;
    std::vector<std::string> to_spawn;
    Uint64 spawned_ticks = 0;

    int wave = 0;
    int foerefs = 0;
};

class FoeSpawnerFactory: public ObjectFactory {
public:
    // e.g., 256,112,1,0 foe_bug;foe_bug,foe_bug,foe_bug;foe_mole
    Object *create(const std::string &options) {
        const char *arr = options.c_str();

        int x = 0, y = 0, spawner_id = 0, animation = 0;
        if (4 != sscanf(arr, "%d,%d,%d,%d", &x, &y, &spawner_id, &animation)) FATAL_ERROR

        // skip until null byte or space
        while (*arr != '\0' && *arr != ' ')
            arr++;

        if (*arr == ' ') arr++; // don't want the last space

        std::vector<std::vector<std::string>> waves;

        // parse waves of enemies
        while (*arr != '\0') {
            char buff[256];
            int i = 0;
            for (; i < sizeof(buff)-1 && *arr != ',' && *arr != ';' && *arr != '\0'; i++)
                buff[i] = *arr++;
            buff[i] = '\0';

            if (i == 0)
                break;

            if (waves.empty())
                waves.push_back(std::vector<std::string>());

            waves.back().push_back(std::string(buff));

            if (*arr == ';')
                waves.push_back(std::vector<std::string>());

            if (*arr != '\0') arr++;
        }

        return new FoeSpawner(x, y, spawner_id, animation, waves);
    }
};

extern FoeSpawner *spawners[];

#endif
