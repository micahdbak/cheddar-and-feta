#ifndef SAVE_STATION_OBJ
#define SAVE_STATION_OBJ "savestation"

#include "object.h"
#include "sprite.h"

class SaveStation : public Object {
public:
    SaveStation(int x, int y);
    ~SaveStation();

    void step() override;

private:
    SDL_FRect dst_rect;
    int x, y;
    Sprite *sprite;
};

class SaveStationFactory : public ObjectFactory {
public:
    SaveStationFactory() {}
    ~SaveStationFactory() {}

    Object *create(std::string options) override {
        int x, y;
        if (sscanf(options.c_str(), "%d,%d", &x, &y) < 2) {
            x = 0, y = 0;
        }
        return new SaveStation(x, y);
    }
};

#endif
