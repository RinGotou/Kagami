#pragma once
#include "object.h"

namespace kagami {
  class Message {
    string value_;
    string detail_;
    int code_;
    shared_ptr<void> object_;
    size_t idx_;

    void Copy(const Message &msg) {
      value_ = msg.value_;
      detail_ = msg.detail_;
      code_ = msg.code_;
      object_ = msg.object_;
      idx_ = msg.idx_;
    }
  public:
    Message() :
      value_(kStrEmpty), 
      detail_(kStrEmpty), 
      code_(kCodeSuccess), 
      idx_(0) {}

    Message(string value, 
      int code, 
      string detail) :
      value_(value), 
      detail_(detail), 
      code_(code), 
      idx_(0) {}

    Message(string detail) :
      value_(kStrRedirect), 
      code_(kCodeSuccess), 
      detail_(detail), 
      idx_(0) {}

    Message(const Message &msg) {
      Copy(msg);
    }

    Message(const Message &&msg) {
      Copy(msg);
    }

    Message &operator=(Message &msg) {
      Copy(msg);
      return *this;
    }

    Message &operator=(Message &&msg) {
      Copy(msg);
      return *this;
    }

    Object GetObj() const;
    void SetObject(Object &object);
    void SetRawString(string str);
    Message &SetValue(const string &value);
    Message &SetCode(const int &code);
    Message &SetDetail(const string &detail);
    Message &SetIndex(const size_t index);

    string GetValue() const { 
      return value_; 
    }

    int GetCode() const { 
      return code_; 
    }

    string GetDetail() const { 
      return detail_; 
    }

    size_t GetIndex() const { 
      return idx_; 
    }

    
  };
}