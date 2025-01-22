#ifndef OBJECT_H
#define OBJECT_H

#include <string>

#include <SDL3/SDL.h>

class Object {
public:
    Object() = default;
    virtual ~Object() = default;

    virtual void step() = 0;
};

class ObjectFactory {
public:
    ObjectFactory() = default;
    virtual ~ObjectFactory() = default;

    virtual Object *create(std::string options) = 0;
};

#endif
