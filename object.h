#ifndef OBJECT_H
#define OBJECT_H

#include <string>

#include <SDL3/SDL.h>

class Object {
public:
    Object() = default;
    virtual ~Object() = default;

    SDL_Texture *texture;
    SDL_FRect *src_rect;
    SDL_FRect dst_rect;

    virtual void step() = 0;
};

class ObjectFactory {
public:
    ObjectFactory() = default;
    virtual ~ObjectFactory() = default;

    virtual Object *create(std::string options) = 0;
};

#endif
