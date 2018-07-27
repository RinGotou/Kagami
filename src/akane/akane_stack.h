#pragma once
#include "akane_list.h"

namespace akane {
  template <class T>
  class stack {
  protected:
    list<T> base;
  public:
    void push(T t) { base.push_back(t); }
    void pop(T t) { base.pop_back(); }
    T &top() { return base.back(); }
    void clear() { base.clear(); }
    size_t size() { return base.size(); }
  };
}