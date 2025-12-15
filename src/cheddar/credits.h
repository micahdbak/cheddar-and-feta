#ifndef CREDITS_OBJ
#define CREDITS_OBJ "credits"

#include "object.h"
#include "sprite.h"

class Credits : public Object {
public:
    Credits();
    ~Credits() = default;

    void step() override;
private:
    std::string tex_id;
    SDL_Texture *texture;
    SDL_FRect src_rect, dst_rect;
};

class CreditsFactory : public ObjectFactory {
public:
    Object *create(const std::string &_) {
        return new Credits();
    };
};

#endif