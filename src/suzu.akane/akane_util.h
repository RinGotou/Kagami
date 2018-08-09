#pragma once
#include <cstddef>
#include <cstring>

namespace akane {
  using std::size_t;
  extern size_t strlen(const char *str);
  extern size_t wstrlen(const wchar_t *str);
}
