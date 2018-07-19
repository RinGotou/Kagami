#pragma once
#include "akane_string.h"

namespace akane {
  class stringbuilder {
    list<char> buf;
  public:
    void append(const char c) {
      buf.push_back(c);
    }

    void append(const char *str) {
      size_t str_size = strlen(str);
      for (size_t i = 0;i < str_size;++i) {
        buf.push_back(str[i]);
      }
    }

    string get() {
      const size_t buf_size = buf.size(); 
      char *res = new char[buf_size + 1];
      for (size_t i = 0;i < buf_size;++i) {
        res[i] = *buf[i];
      }
      res[buf_size] = '\0';
      return string(res);
    }

    stringbuilder() {}
    stringbuilder(stringbuilder &builder) { this->buf = builder.buf; }
    stringbuilder(const char *str) { append(str); }
  };

  class wstringbuilder {
    list<wchar_t> buf;
  public:
    void append(const wchar_t c) {
      buf.push_back(c);
    }

    void append(const wchar_t *str) {
      size_t str_size = wstrlen(str);
      for (size_t i = 0;i < str_size;++i) {
        buf.push_back(str[i]);
      }
    }

    wstring get() {
      const size_t buf_size = buf.size(); 
      wchar_t *res = new wchar_t[buf_size + 1];
      for (size_t i = 0;i < buf_size;++i) {
        res[i] = *buf[i];
      }
      res[buf_size] = L'\0';
      return wstring(res);
    }

    wstringbuilder() {}
    wstringbuilder(const wchar_t *str) { append(str); }
  };
}
