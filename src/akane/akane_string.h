#pragma once
#include "akane_util.h"

namespace akane {
  //A simple string container.
  //thanks to https://github.com/adolli/FruitString !
  template <class T>
  class basic_string_container {
  protected:
    T *data;
    size_t size_;
  public:
    basic_string_container() : data(nullptr), size_(0) {}
    basic_string_container(const T *data, size_t size) : size_(size){
      this->data = new T[size + 1];
      memcpy(this->data, data, sizeof(T)*size);
    }
    basic_string_container(const T unit) : size_(1) {
      this->data = new T[2];
      data[0] = unit;
    }
    basic_string_container(basic_string_container &bsc) : size_(bsc.size_) {
      data = new T[bsc.size_ + 1];
      memcpy(data, bsc.data, sizeof(T)*bsc.size_);
    }
    basic_string_container &operator=(basic_string_container &bsc) {
      if (&bsc == this) return *this;
      delete[] data;
      this->size_ = bsc.size_;
      data = new T[bsc.size_ + 1];
      memcpy(data, bsc.data, sizeof(T)*bsc.size_);
      return *this;
    }
    basic_string_container &operator=(basic_string_container &&bsc) {
      if (&bsc == this) return *this;
      delete[] data;
      this->size_ = bsc.size_;
      data = new T[bsc.size_ + 1];
      memcpy(data, bsc.data, sizeof(T)*bsc.size_);
      return *this;
    }
    
    T &get(size_t pos) { return data[pos]; } 
    ~basic_string_container() { delete[] data; }
    void clear() { delete[] data; size_ = 0; }
    size_t size() const { return size_; }
  };

  template <class T>
  class basic_string{
  protected:
    using _ContainerType = basic_string_container<T>;
    basic_string_container<T> container;
  public:
    basic_string() {}
    basic_string(const T *t, size_t size) : container(t, size) {}
    basic_string(basic_string &str) : container(str.container) {}
    basic_string(basic_string &&str) : container(str.container) {}
    T &at(size_t pos) { return container.get(pos); }
    T &operator[](size_t pos) { return container.get(pos); }
    size_t size() const { return container.size(); }
    void clear() { container.clear(); }
  };


  class string : public basic_string<char> {
    const char kEndingSymbol = '\0';
  public:
    string() : basic_string<char>() { container.get(0) = kEndingSymbol; }
    string(const char *str) : basic_string<char>(str, strlen(str)) { container.get(strlen(str)) = kEndingSymbol; }
    string(string &str) : basic_string<char>(str) { container.get(container.size()) = kEndingSymbol; }
    string(string &&str) : basic_string<char>(str) { container.get(container.size()) = kEndingSymbol; }
    bool compare(const char *str) {
      size_t str_size = strlen(str);
      if (container.size() != str_size) return false;
      size_t pos = 0;
      while (pos < container.size()) {
        if (str[pos] != container.get(pos)) return false;
        ++pos;
      }
      return true;
    }
    bool compare(string &str) {
      if (str.container.size() != container.size()) return false;
      size_t pos = 0;
      while(pos < container.size()) {
        if (str.container.get(pos) != container.get(pos)) return false;
        ++pos;
      }
      return true;
    }
    bool operator==(string &str) {
      return compare(str);
    }
    bool operator==(const char *str) {
      return compare(str);
    }
    string &operator=(string &str) {
      container = str.container;
      container.get(container.size()) = kEndingSymbol;
      return *this;
    }
    string &operator=(const char *str) {
      size_t len = strlen(str);
      container = _ContainerType(str, len);
      container.get(len) = kEndingSymbol;
      return *this;
    }
    const char *c_str() {
      return &container.get(0);
    }
  };

  class wstring : public basic_string<wchar_t> {
    const wchar_t kEndingSymbol = L'\0';
  public:
    wstring() : basic_string<wchar_t>() { container.get(0) = kEndingSymbol; }
    wstring(const wchar_t *str) : basic_string<wchar_t>(str, wstrlen(str)) { container.get(wstrlen(str)) = kEndingSymbol; }
    wstring(wstring &str) : basic_string<wchar_t>(str) { container.get(container.size()) = kEndingSymbol; }
    wstring(wstring &&str) : basic_string<wchar_t>(str) { container.get(container.size()) = kEndingSymbol; }
    bool compare(const wchar_t *str) {
      size_t str_size = wstrlen(str);
      if (container.size() != str_size) return false;
      size_t pos = 0;
      while (pos < container.size()) {
        if (str[pos] != container.get(pos)) return false;
        ++pos;
      }
      return true;
    }
    bool compare(wstring &str) {
      if (str.container.size() != container.size()) return false;
      size_t pos = 0;
      while(pos < container.size()) {
        if (str.container.get(pos) != container.get(pos)) return false;
        ++pos;
      }
      return true;
    }
    bool operator==(wstring &str) {
      return compare(str);
    }
    bool operator==(const wchar_t *str) {
      return compare(str);
    }
    wstring &operator=(wstring &str) {
      container = str.container;
      container.get(container.size()) = kEndingSymbol;
      return *this;
    }
    wstring &operator=(const wchar_t *str) {
      size_t len = wstrlen(str);
      container = _ContainerType(str, len);
      container.get(len) = kEndingSymbol;
      return *this;
    }
    const wchar_t *c_str() {
      return &container.get(0);
    }
  };
}

