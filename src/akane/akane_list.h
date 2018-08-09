#pragma once
#include "akane_util.h"

namespace akane {
    //Node class for linked list
  template <class T>
  class sl_node {
  public:
    T data;
    sl_node *next;
    sl_node() { next = nullptr; }
    sl_node(T &t) { data = t; next = nullptr; }
    sl_node(T &&t) { data = t; next = nullptr; }
    sl_node(sl_node &sln) { this->data = sln.data; this->next = sln.next; }
  };

  class ref_blk {
  public:
    size_t count;
    ref_blk() : count(1) {}
  };

  //Simple linked list base with refenrence count.
  template <class T>
  class list {
    using _DataNode = sl_node<T>;
    ref_blk *blk;
    size_t count;
    _DataNode *root;
    _DataNode *tail;
    bool health;
  public:
    list() : root(nullptr), 
    tail(nullptr), 
    count(0),
    health(true) {
      blk = new ref_blk();
    }

    list(list &lst) : blk(lst.blk), count(lst.count), root(lst.root), 
    tail(lst.tail) {
      blk->count++;
      this->health = lst.health;
    }

    ~list() {
      blk->count--;
      if (blk->count == 0) {
        delete blk;
        if (root == nullptr && tail == nullptr) return;
        if (root != tail) {
          _DataNode *ptr = nullptr;
          while (root != nullptr) {
            ptr = root;
            root = root->next;
            delete ptr;
          }
        }
        else if (count == 1){
          delete root;
        }
      }
    }

    list &operator=(list &lst) {
      ++lst.blk->count;
      --this->blk->count;
      if (this->blk->count == 0) {
        clear();
      }
      this->blk = lst.blk;
      count = lst.count;
      root = lst.root;
      tail = lst.tail;
      health = lst.health;
      return *this;
    }

    void push_back(T t) {
      if (root == nullptr) {
        root = new _DataNode(t);
        if (root == nullptr) health = false;
        tail = root;
      }
      else {
        tail->next = new _DataNode(t);

        if (tail->next != nullptr) {
          tail = tail->next;
          tail->next = nullptr;
        }
        else health = false;
      }
      ++count;
    }

    void pop_back() {
      if (tail == nullptr) return;
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

    void push_front(T t) {
      if (root == nullptr) {
        root = new _DataNode(t);
        if (root == nullptr) health = false;
        tail = root;
      }
      else {
        _DataNode *ptr = new _DataNode(t);
        if (ptr == nullptr) return;
        ptr->next = root;
        root = ptr;
      }
      ++count;
    }

    void pop_front() {
      if (root == nullptr) return;
      if (root == tail) {
        delete root;
        root = nullptr;
        tail = nullptr;
      }
      else {
        _DataNode *ptr = root;
        root = root->next;
        delete ptr;
      }
      --count;
    }

    T &at(size_t pos) {
      size_t sub = 0;
      _DataNode *ptr = root;
      while (sub < pos) {
        ++sub;
        ptr = ptr->next;
      }
      return ptr->data;
    }

    void clear() {
      if (root == nullptr && tail == nullptr) return;
      if (root != tail) {
        _DataNode *ptr = nullptr;
        while (root != nullptr) {
          ptr = root;
          root = root->next;
          delete ptr;
        }
        root = nullptr;
        tail = nullptr;
      }
      else if (count == 1){
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

    void insert(size_t pos, T t) {
      if (pos > count - 1) {
        tail->next = new _DataNode(t);
        ++count;
        tail = tail->next;
      }
      else {
        size_t sub = 0;
        _DataNode *ptr = root, *forward = nullptr;
        while(sub < pos) {
          ++sub;
          forward = ptr;
          ptr = ptr->next;
          if (ptr == nullptr) break;
        }
        _DataNode *newNode = new _DataNode(t);
        forward->next = newNode;
        newNode->next = ptr;
        ++count;
      }
    }

    void copy(list &lst) {
      _DataNode *ptr = lst.root;
      while (ptr != nullptr) {
        this->push_back(ptr->data);
        ptr = ptr->next;
      }
    }

    T &operator[](size_t pos) { return at(pos); }
    T &back() { return tail->data; }
    T &front() { return root->data; }
    size_t ref_count() const { return blk->count; }
    bool empty() const { return (root == nullptr && tail == nullptr && count == 0); }
    bool good() const { return health; }
    size_t size() const { return count; }
  };
}