#include "akane_string.h"

namespace akane {
  bool operator==(const akane::string& lhs, const akane::string& rhs) {
  return lhs.compare(rhs);
  }

  bool operator!=(const akane::string& lhs, const akane::string& rhs) {
  return !lhs.compare(rhs);
  }

  std::ostream &operator<<(std::ostream &os, const akane::string &str) {
    os << str.c_str();
    return os;
  }
}


