#pragma once
#include "parser.h"

namespace kagami {
  class FileStreamWrapper {
  private:
    shared_ptr<void> ptr;
  public:

  };

  template <class T>
  shared_ptr<void> SimpleSharedPtrCopy(shared_ptr<void> target) {
    T temp(*static_pointer_cast<T>(target));
    return make_shared<T>(temp);
  }
}