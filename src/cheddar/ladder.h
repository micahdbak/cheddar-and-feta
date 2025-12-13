#ifndef LADDER_OBJ
#define LADDER_OBJ "ladder"

#include <iostream>

#include "object.h"
#include "sprite.h"

class Ladder : public Object {
public:
    Ladder(int x, int y, std::string next_map, int which_coord);
    ~Ladder();

    void step() override;

private:
    float x, y;
    int which_coord;
    std::string next_map;
};

class LadderFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y, which_coord;
        sscanf(options.c_str(), "%d,%d,%d", &x, &y, &which_coord);

        const char *map = options.c_str();
        while (*map != '\0' && *map != ' ')
            map++;

        if (*map == ' ') map++; // don't want the last space

        if (*map == '\0') {
            // empty map
            std::cerr << "LadderFactory::create: no map provided" << std::endl;
            std::exit(1);
        }

        return new Ladder(x, y, std::string(map), which_coord);
    }
};

#endif