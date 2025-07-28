#ifndef ITEM_CHEESE
#define ITEM_CHEESE "item_cheese"

#include "object.h"
#include "item.h"

// dropped cheese
class Cheese : public Object {
public:
    static std::string Options(int x, int y, int amount) {
        char buff[256];
        snprintf(buff, sizeof(buff), "%d,%d,%d", x, y, amount);
        return std::string(buff);
    }

    Cheese(float x, float y, int amount);
    ~Cheese();

    void step() override;

private:
    void render_cheese();

    std::string tex_id;
    SDL_Texture *texture = nullptr;
    SDL_FRect dst_rect;
    float x, y;
    int amount, depth_offset;
};

class CheeseFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x = 0, y = 0, amount = 1;
        if (sscanf(options.c_str(), "%d,%d,%d", &x, &y, &amount) < 3) {
            amount = 1;
        }
        return new Cheese(float(x), float(y), amount);
    }
};

#endif
