#pragma once
#include "util.h"
#include "object.h"

namespace kagami {
  class Message {
  private:
    StateLevel level_;
    string detail_;
    StateCode code_;
    shared_ptr<void> object_;
    size_t idx_;

  public:
    Message() :
      level_(kStateNormal), 
      detail_(""), 
      code_(kCodeSuccess), 
      idx_(0) {}

    Message(const Message &msg) :
      level_(msg.level_),
      detail_(msg.detail_),
      code_(msg.code_),
      object_(msg.object_),
      idx_(msg.idx_) {}

    Message(const Message &&msg) :
      Message(msg) {}

    Message(StateCode code, string detail, StateLevel level = kStateNormal) :
      level_(level), 
      detail_(detail), 
      code_(code), 
      idx_(0) {}

    Message(string detail) :
      level_(kStateNormal), 
      code_(kCodeObject), 
      detail_(""), 
      object_(make_shared<Object>(detail)),
      idx_(0) {}

    Message &operator=(Message &msg) {
      level_ = msg.level_;
      detail_ = msg.detail_;
      code_ = msg.code_;
      object_ = msg.object_;
      idx_ = msg.idx_;
      return *this;
    }

    Message &operator=(Message &&msg) {
      return this->operator=(msg);
    }

    StateLevel GetLevel() const {
      return level_;
    }

    StateCode GetCode() const { 
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

    Message &SetLevel(StateLevel level) {
      level_ = level;
      return *this;
    }

    Message &SetCode(StateCode code) {
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
  };
}