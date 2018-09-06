#pragma once
#include "machine.h"

namespace kagami {
  const string kArrayBaseMethods = "size|__at|__print";
  const string kStringMethods = "size|__at|__print|substr|to_wide";
  const string kInStreamMethods = "get|good|getlines|close|eof";
  const string kOutStreamMethods = "write|good|close";
  const string kRegexMethods = "match";

  using ArrayBase = vector<Object>;
  template <class T>
  shared_ptr<void> SimpleSharedPtrCopy(shared_ptr<void> target) {
    T temp(*static_pointer_cast<T>(target));
    return make_shared<T>(temp);
  }
}