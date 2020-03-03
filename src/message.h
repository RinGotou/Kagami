#pragma once
#include "lexical.h"
#include "object.h"

namespace kagami {
  enum StateLevel {
    kStateNormal,
    kStateError,
    kStateWarning
  };

  class Message {
  private:
    bool invoking_msg_;
    StateLevel level_;
    string detail_;
    optional<Object> obj_;
    size_t idx_;

  public:
    Message() :
      invoking_msg_(false),
      level_(kStateNormal), 
      detail_(""), 
      obj_(std::nullopt),
      idx_(0) {}

    Message(const Message &msg) :
      invoking_msg_(msg.invoking_msg_),
      level_(msg.level_),
      detail_(msg.detail_),
      obj_(msg.obj_),
      idx_(msg.idx_) {}

    Message(const Message &&msg) :
      Message(msg) {}

    Message(string detail, StateLevel level = kStateNormal) :
      invoking_msg_(false),
      level_(level), 
      detail_(detail), 
      idx_(0) {}

    Message &operator=(Message &msg) {
      invoking_msg_ = msg.invoking_msg_;
      level_ = msg.level_;
      detail_ = msg.detail_;
      obj_ = msg.obj_;
      idx_ = msg.idx_;
      return *this;
    }

    Message &operator=(Message &&msg) {
      return this->operator=(msg);
    }

    StateLevel GetLevel() const { return level_; }
    string GetDetail() const { return detail_; }
    size_t GetIndex() const { return idx_; }
    bool HasObject() const { return obj_.has_value(); }
    //bool HasObject() const { return object_ != nullptr; }
    bool IsInvokingMsg() const { return invoking_msg_; }

    Object GetObj() const {
      if (!obj_.has_value()) return Object();
      return obj_.value();
    }

    Message &SetObject(Object &object) {
      obj_ = object;
      return *this;
    }

    Message &SetObject(Object &&object) {
      return this->SetObject(object);
    }

    Message &SetObject(bool value) {
      obj_ = Object(value, kTypeIdBool);
      return *this;
    }

    Message &SetObject(int64_t value) {
      obj_ = Object(value, kTypeIdInt);
      return *this;
    }

    Message &SetObject(double value) {
      obj_ = Object(value, kTypeIdFloat);
      return *this;
    }

    Message &SetObject(string value) {
      obj_ = Object(value, kTypeIdString);
      return *this;
    }

    Message &SetLevel(StateLevel level) {
      level_ = level;
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

    Message &SetInvokingSign(Object &obj) {
      obj_ = obj;
      invoking_msg_ = true;
      return *this;
    }

    Message &SetInvokingSign(Object &&obj) {
      return this->SetInvokingSign(obj);
    }

    void Clear() {
      level_ = kStateNormal;
      detail_.clear();
      detail_.shrink_to_fit();
      obj_.reset();
      idx_ = 0;
    }
  };

  using CommentedResult = tuple<bool, string>;
}