#pragma once

namespace akane {
  template <class T>
  class node {
  public:
    T data;
    node *next;
    node() { next = nullptr; }
    node(T &t) { data = t; next = nullptr;  }
    node(T &&t) { data = t; next = nullptr;  }
  };

  template <class T>
  class list {
    using _DataNode = node<T>;
    size_t count;
    _DataNode *root;
    _DataNode *tail;
    bool health;
  public:
    list() { root = nullptr; count = 0; health = true; }
    bool empty() const { return (root == nullptr && count == 0); }
    bool good() const { return health; }
    size_t size() const { return count; }
    void push_back(T t) {
      if (root == nullptr) {
        root = new _DataNode(t);
        if (root == nullptr) health = false;
        tail = root;
        ++count;
      }
      else {
        tail->next = new _DataNode(t);
        ++count;
        if (tail->next != nullptr) {
          tail = tail->next;
          tail->next = nullptr;
        }
        else health = false;
      }
    }
    void pop_back() {
      if (root != tail) {
        _DataNode *ptr = root;
        while (ptr->next != tail) ptr = ptr->next;
        delete tail;
        tail = ptr;
        tail->next = nullptr;
        --count;
      }
      else {
        delete root;
        root = nullptr;
        tail = nullptr;
        count = 0;
      }
    }
    T *at(size_t pos) {
      if (pos > count - 1) return nullptr;
      if (empty()) return nullptr;
      size_t sub = 0;
      _DataNode *walker = root;
      while (sub < pos) {
        ++sub;
        walker = walker->next;
        if (walker == nullptr) break;
      }
      return &walker->data;
    }
    T *operator[](size_t pos) {
      return at(pos);
    }
    T *back() {
      if (tail != nullptr) {
        return &tail->data;
      }
      return nullptr;
    }
    T *front() {
      if (root != nullptr) {
        return &root->data;
      }
      return nullptr;
    }
    void clear() {
      if (root != tail) {
        _DataNode *ptr = nullptr;
        while (root != nullptr) {
          ptr = root;
          root = root->next;
          delete ptr;
        }
      }
      else {
        delete root;
        root = nullptr;
        tail = nullptr;
      }
      count = 0;
    }
    void replace(size_t pos, T t) {
      if (pos > count - 1) return;
      if (empty()) return;
      size_t sub = 0;
      _DataNode *ptr = root;
      while (sub < pos) {
        ++sub;
        ptr = ptr->next;
      }
      if (ptr != nullptr) ptr->data = t;
    }
    size_t find(T t) {
      size_t sub = 0;
      _DataNode *ptr = root;
      while (ptr != nullptr) {
        if (ptr->data == t) break;
        ++sub;
        ptr = ptr->next;
      }
      return sub;
    }
    void erase(size_t pos) {
      if (pos > count - 1) return;
      if (empty()) return;
      size_t sub = 0;
      _DataNode *ptr = root, *forward = nullptr;
      while (sub < pos) {
        ++sub;
        forward = ptr;
        ptr = ptr->next;
      }
      if (forward != nullptr) forward->next = ptr->next;
      delete ptr;
      --count;
    }
  };

  template <class T>
  class basic_string_container {
    T *data;
    size_t size;
  public:

  };
}