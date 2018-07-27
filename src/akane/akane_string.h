#pragma once
#include "akane_util.h"
#include <iostream>
#include <cstdlib>

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
    basic_string_container(const T *data, size_t size) : size_(size) {
      this->data = new T[size + 1];
      memcpy(this->data, data, sizeof(T)*size);
    }
    basic_string_container(const T unit) : size_(1) {
      this->data = new T[2];
      data[0] = unit;
    }
    basic_string_container(const basic_string_container &bsc) : size_(bsc.size_) {
      data = new T[bsc.size_ + 1];
      memcpy(data, bsc.data, sizeof(T)*bsc.size_);
    }
    basic_string_container &operator=(const basic_string_container &bsc) {
      if (&bsc == this) return *this;
      delete[] data;
      this->size_ = bsc.size_;
      data = new T[bsc.size_ + 1];
      memcpy(data, bsc.data, sizeof(T)*bsc.size_);
      return *this;
    }
    basic_string_container &operator=(const basic_string_container &&bsc) {
      if (&bsc == this) return *this;
      delete[] data;
      this->size_ = bsc.size_;
      data = new T[bsc.size_ + 1];
      memcpy(data, bsc.data, sizeof(T)*bsc.size_);
      return *this;
    }
    
    T &get(size_t pos) const { return data[pos]; } 
    T &_get_unsafe(size_t pos) { return data[pos]; }
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
    basic_string(const basic_string &str) : container(str.container) {}
    basic_string(const basic_string &&str) : container(str.container) {}
    T &at(size_t pos) const { return container.get(pos); }
    T &operator[](size_t pos) { return container.get(pos); }
    size_t size() const { return container.size(); }
    void clear() { container.clear(); }
  };

  //Light string class.
  //For building work,please use stringbuilder class.
  class string : public basic_string<char> {
    const char kEndingSymbol = '\0';
  public:
    string() : basic_string<char>() { container._get_unsafe(0) = kEndingSymbol; }
    string(const char *str) : basic_string<char>(str, strlen(str)) { container._get_unsafe(strlen(str)) = kEndingSymbol; }
    string(const string &str) : basic_string<char>(str) { container._get_unsafe(container.size()) = kEndingSymbol; }
    string(const string &&str) : basic_string<char>(str) { container._get_unsafe(container.size()) = kEndingSymbol; }
    string(const std::string &str) : basic_string<char>(str.c_str(), str.size()) {}
    bool compare(const char *str) const {
      size_t str_size = strlen(str);
      if (container.size() != str_size) return false;
      size_t pos = 0;
      while (pos < container.size()) {
        if (str[pos] != container.get(pos)) return false;
        ++pos;
      }
      return true;
    }
    bool compare(const string &str) const {
      if (str.container.size() != container.size()) return false;
      size_t pos = 0;
      while(pos < container.size()) {
        if (str.container.get(pos) != container.get(pos)) return false;
        ++pos;
      }
      return true;
    }
    string &operator=(const string &str) {
      container = str.container;
      container._get_unsafe(container.size()) = kEndingSymbol;
      return *this;
    }
    string &operator=(const char *str) {
      size_t len = strlen(str);
      container = _ContainerType(str, len);
      container._get_unsafe(len) = kEndingSymbol;
      return *this;
    }
    char *c_str() const {
      return &container.get(0);
    }
    string substr(size_t pos, size_t len) {
      char *res = new char[len + 1];
      for(size_t i = 0;i < len;++i) {
        res[i] = container.get(i + pos);
      }
      res[len] = kEndingSymbol;
      return string(res);
    }
    bool is_blank() const {
      for(size_t i = 0;i < container.size();++i) {
        char c = container.get(i);
        if(c != ' ' && c !=  '\t' && c != '\r' && c != '\n') {
          return false;
        }
      }
      return true;
    }
    int to_int() const { return atoi(&container.get(0)); }
    float to_float() const { return float(atof(&container.get(0))); }
    double to_double() const { return atof(&container.get(0)); }
    std::string to_std_string() const { return std::string(&container.get(0)); }
    char front() const { return container.get(0); }
    char back() const { return container.get(container.size() - 1); }
    bool empty() const { return (container.size() == 0); }
    friend bool operator==(const akane::string& lhs, const akane::string& rhs);
    friend bool operator!=(const akane::string& lhs, const akane::string& rhs);
    friend std::ostream &operator<<(std::ostream &os, const akane::string &str);
  };
}

