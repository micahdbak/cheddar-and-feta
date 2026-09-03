#ifndef OBJECT_H
#define OBJECT_H

#include <SDL3/SDL.h>

#include <memory>
#include <string>

extern int _object_id_counter;  // main.cpp

class Object {
 public:
  Object() {
    this->id = _object_id_counter++;
    this->deleted_ptr = std::make_shared<bool>(false);
  };
  virtual ~Object() { *this->deleted_ptr = true; }

  virtual void step() = 0;
  virtual void save_data() {}

  std::shared_ptr<bool> get_deleted_ptr() { return this->deleted_ptr; }

  int id;

 private:
  // as this is a shared_ptr, it will persist after the object is deleted,
  // assuming something else has an instance of it from Object::get_deleted_ptr.
  // CHECK AGAINST IT IF YOU ARE USING A POINTER TO AN OBJECT.
  std::shared_ptr<bool> deleted_ptr;
};

class ObjectFactory {
 public:
  ObjectFactory() = default;
  virtual ~ObjectFactory() = default;

  virtual Object* create(const std::string& options) = 0;
};

#endif
