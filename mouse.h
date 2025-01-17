#ifndef MOUSE_OBJ
#define MOUSE_OBJ "mouse"

#include "object.h"
#include "sprite.h"

class Mouse : public Object {
public:
    Mouse(int x, int y);
    ~Mouse();

    void step() override;

private:
    Sprite *sprite;
};

class MouseFactory : public ObjectFactory {
public:
    MouseFactory() {}
    ~MouseFactory() {}

    Object *create(std::string options) override {
        int x, y;
        if (sscanf(options.c_str(), "%d,%d", &x, &y) < 2) {
            x = 0, y = 0;
        }
        return new Mouse(x, y);
    }
};

#endif
