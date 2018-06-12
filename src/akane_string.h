#pragma once

namespace akane {
    size_t strlen(const char *str) {
    size_t len = 0;
    while (*(str + len) != '\0') {
      ++len;
    }
    return len;
  }

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
      data = new T[bsc.size + 1];
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
    
    T &get(size_t pos) { return data[pos]; } 
    ~basic_string_container() { delete[] data; }
    void clear() { delete[] data; size_ = 0; }
    size_t size() const { return size_; }
  };

  class string {
  protected:
    using char_container = basic_string_container<char>;
    const char kEnd = '\0';
    char_container container;
  public:
    string() {}
    string(const char *str) : container(str, strlen(str)) { container.get(strlen(str)) = kEnd; }
    string(string &str) { this->container = str.container; container.get(container.size()) = kEnd; }
    string(string &&str) { this->container = str.container; container.get(container.size()) = kEnd; }
    char &at(size_t pos) { return container.get(pos); }
    char &operator[](size_t pos) { return at(pos); }
    size_t size() const { return container.size(); }
    void clear() { container.clear(); }

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

    string &operator=(string &str) {
      container = str.container;
      container.get(container.size()) = kEnd;
      return *this;
    }

    char *c_str() {
      return &container.get(0);
    }
  };

  class string_builder {
    
  };
}

