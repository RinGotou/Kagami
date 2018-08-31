#pragma once
#include "machine.h"
#include "basic_string.h"

namespace kagami {
  template <class T>
  shared_ptr<void> SimpleSharedPtrCopy(shared_ptr<void> target) {
    T temp(*static_pointer_cast<T>(target));
    return make_shared<T>(temp);
  }

  class Array : public basic_string<Object> {
  public:
    Array(const Object *object, size_t size) : basic_string<Object>(object, size) {}
    Array(const Array &ar) : basic_string<Object>(ar) {}
    Array(const Array &&ar) : basic_string<Object>(ar) {}
    //Array(size_t size)
  };
}