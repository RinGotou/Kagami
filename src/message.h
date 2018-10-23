#pragma once
#include "util.h"
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
      value_(kStrEmpty), 
      code_(kCodeObject), 
      detail_(kStrEmpty), 
      idx_(0) {

      object_ = 
        make_shared<Object>(detail, util::GetTokenType(detail));
    }

    Message(const Message &msg) {
      value_ = msg.value_;
      detail_ = msg.detail_;
      code_ = msg.code_;
      object_ = msg.object_;
      idx_ = msg.idx_;
    }

    Message(const Message &&msg) {
      value_ = msg.value_;
      detail_ = msg.detail_;
      code_ = msg.code_;
      object_ = msg.object_;
      idx_ = msg.idx_;
    }

    Message &operator=(Message &msg) {
      Copy(msg);
      return *this;
    }

    Message &operator=(Message &&msg) {
      Copy(msg);
      return *this;
    }

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

    Object GetObj() const {
      return *static_pointer_cast<Object>(object_);
    }

    Message &SetObject(Object &object) {
      object_ = make_shared<Object>(object);
      code_ = kCodeObject;
      return *this;
    }

    Message &SetObject(Object &&object) {
      return this->SetObject(object);
    }

    void SetRawString(string str) {
      value_ = kStrRedirect;
      code_ = kCodeSuccess;
      detail_ = str;
    }

    Message &SetValue(const string &value) {
      value_ = value;
      return *this;
    }

    Message &SetCode(const int &code) {
      code_ = code;
      return *this;
    }

    Message &SetDetail(const string &detail) {
      detail_ = detail;
      return *this;
    }

    Message &SetIndex(const size_t index) {
      idx_ = index;
      return *this;
    }

    void Get(string *value, int *code, string *detail) {
      if (value != nullptr) *value = value_;
      if (code != nullptr) *code = code_;
      if (detail != nullptr) *detail = detail_;
    }
  };
}