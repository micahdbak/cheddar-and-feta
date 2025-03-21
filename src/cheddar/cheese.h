#ifndef CHEESE_OBJ
#define CHEESE_OBJ "cheese"

#include "object.h"

class Cheese : public Object {
public:
    Cheese(float x, float y, int amount); // x,y,amount
    ~Cheese();

    void step() override;

private:
    void render_cheese();

    SDL_Texture *texture = nullptr;
    SDL_FRect dst_rect;
    float x, y;
    int amount, depth_offset;
};

class CheeseFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int x = 0, y = 0, amount = 1;
        sscanf(options.c_str(), "%d,%d,%d", &x, &y, &amount);
        return new Cheese(float(x), float(y), amount);
    }
};

#endif
