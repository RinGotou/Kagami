/*
 * Akane - simple template kit 
 */
#pragma once
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

  //Node class for double linked list
  template <class T>
  class node {
  public:
    T data;
    node *next, *forward;
    node() { next = nullptr; forward = nullptr; }
    node(T &t) { data = t; next = nullptr; forward = nullptr; }
    node(T &&t) { data = t; next = nullptr; forward = nullptr; }
    node(node &n) { data = n.data; next = nullptr; forward = nullptr; }
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
    list(list &lst) : blk(lst.blk), 
    count(lst.count), 
    root(lst.root), 
    tail(lst.tail), 
    health(lst.health) {
      blk->count++;
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
      lst.blk->count++;
      this->blk->count--;
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
    bool empty() const { return (root == nullptr && count == 0); }
    bool good() const { return health; }
    size_t size() const { return count; }
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
    size_t ref_count() const { return blk->count; }
  };


  //A simple string container.
  //thanks to https://github.com/adolli/FruitString !
  template <class T>
  class basic_string_container {
  protected:
    T *data;
    size_t size_;
  public:
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
      memcpy(data, bsc.data, bsc.size_);
    }
    basic_string_container &operator=(basic_string_container &bsc) {
      if (&bsc == this) return this;
      this->size_ = bsc.size_;
      //TODO:???
    }
    
    T &get(size_t pos) { return data[pos]; } 
    ~basic_string_container() { delete[] data; }
    void clear() { delete[] data; }
    size_t size() const { return size_; }
  };

  class string : public basic_string_container<char> {
    
  };
}