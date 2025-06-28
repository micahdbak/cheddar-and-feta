#ifndef OBJECT_H
#define OBJECT_H

#include <string>

#include <SDL3/SDL.h>

extern int _object_id_counter; // main.cpp

class Object {
public:
    Object() {
        this->id = _object_id_counter++;
    };
    virtual ~Object() = default;

    virtual void step() = 0;

    virtual void save_data() {}
    virtual void post_save_data() {}

    int id;
};

class ObjectFactory {
public:
    ObjectFactory() = default;
    virtual ~ObjectFactory() = default;

    virtual Object *create(const std::string &options) = 0;
};

#endif
