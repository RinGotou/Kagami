#include "akane.h"

std::ostream &operator<<(std::ostream &os, akane::string &str) {
  os << str.c_str();
  return os;
}

std::ostream &operator<<(std::ostream &os, akane::wstring &str) {
  os << str.c_str();
  return os;
}