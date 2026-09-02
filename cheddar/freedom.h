#ifndef FREEDOM_OBJ
#define FREEDOM_OBJ "freedom"

#include "object.h"
#include "game.h"

class Freedom : public Object {
public:
    Freedom(int x, int y);
    ~Freedom();

    void step() override;

private:
    int x, y;
    Uint64 timer_start = 0;
};

class FreedomFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x = 0, y = 0;
        if (2 != sscanf(options.c_str(), "%d,%d", &x, &y)) FATAL_ERROR

        return new Freedom(x, y);
    }
};

#endif