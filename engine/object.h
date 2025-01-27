#ifndef OBJECT_H
#define OBJECT_H

#include <string>

#include <SDL3/SDL.h>

class Object {
public:
    Object() = default;
    virtual ~Object() = default;

    virtual void step() = 0;

    virtual void save_data() {}
    virtual void post_save_data() {}
};

class ObjectFactory {
public:
    ObjectFactory() = default;
    virtual ~ObjectFactory() = default;

    virtual Object *create(const std::string &options) = 0;
};

#endif
