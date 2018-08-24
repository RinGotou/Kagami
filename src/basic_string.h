#pragma once
#include <cstddef>

namespace kagami {
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
  class basic_string {
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
}