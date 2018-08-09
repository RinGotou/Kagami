#pragma once
#include "akane_string.h"

namespace akane {
  class stringbuilder {
    list<char> buf;
  public:
    stringbuilder &append(const char c) {
      buf.push_back(c);
      return *this;
    }

    stringbuilder &append(const char *str) {
      size_t str_size = strlen(str);
      for (size_t i = 0;i < str_size;++i) {
        buf.push_back(str[i]);
      }
      return *this;
    }

    char back() {
      return buf.back();
    }

    string get() {
      const size_t buf_size = buf.size(); 
      char *res = new char[buf_size + 1];
      for (size_t i = 0;i < buf_size;++i) {
        res[i] = buf[i];
      }
      res[buf_size] = '\0';
      return string(res);
    }

    string move() {
      const size_t buf_size = buf.size();
      char *res = new char[buf_size + 1];
      size_t pos = 0;
      while(pos < buf_size && !buf.empty()) {
        res[pos] = buf.front();
        buf.pop_front();
        ++pos;
      }
      res[buf_size] = '\0';
      return string(res);
    }

    void clear() { buf.clear(); }
    size_t size() { return buf.size(); }
    stringbuilder() {}
    stringbuilder(stringbuilder &builder) { this->buf = builder.buf; }
    stringbuilder(const char *str) { append(str); }
  };
}
