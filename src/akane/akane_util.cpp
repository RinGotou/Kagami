#include "akane_util.h"

namespace akane {
  inline size_t strlen(const char *str) {
    size_t len = 0;
    while (*(str + len) != '\0') {
      ++len;
    }
    return len;
  }

  inline size_t wstrlen(const wchar_t *str) {
    size_t len = 0;
    while (*(str + len) != '\0') {
      ++len;
    }
    return len;
  }
}