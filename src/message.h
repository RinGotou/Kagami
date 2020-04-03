#pragma once
#include "lexical.h"
#include "object.h"

namespace kagami {
  enum StateLevel {
    kStateNormal,
    kStateError,
    kStateWarning
  };

  struct ObjectPrototype {
    ObjectInfo info;
    shared_ptr<void> ptr;
  };

  class Message {
  private:
    bool invoking_msg_;
    StateLevel level_;
    string detail_;
    optional<ObjectPrototype> slot_;
    size_t idx_;

  public:
    Message() :
      invoking_msg_(false),
      level_(kStateNormal), 
      detail_(""), 
      slot_(std::nullopt),
      idx_(0) {}

    Message(const Message &msg) :
      invoking_msg_(msg.invoking_msg_),
      level_(msg.level_),
      detail_(msg.detail_),
      slot_(msg.slot_),
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
      slot_ = msg.slot_;
      idx_ = msg.idx_;
      return *this;
    }

    Message &operator=(Message &&msg) {
      return this->operator=(msg);
    }

    StateLevel GetLevel() const { return level_; }
    string GetDetail() const { return detail_; }
    size_t GetIndex() const { return idx_; }
    //bool HasObject() const { return obj_.has_value(); }
    bool HasObject() const { return slot_.has_value(); }
    bool IsInvokingRequest() const { return invoking_msg_; }

    Object GetObj() const {
      if (slot_.has_value()) {
        auto &prototype = slot_.value();
        return Object(prototype.info, prototype.ptr);
      }

      return Object();
    }

    //ObjectInfo GetObjectInfo() const {
    //  return 
    //}

    Message &SetObject(Object &object) {
      slot_ = ObjectPrototype{ object.GetObjectInfoTable(), object.Get() };
      return *this;
    }

    Message &SetObject(Object &&object) {
      return this->SetObject(object);
    }

    Message &SetObjectRef(Object &obj) {
      slot_ = ObjectPrototype{
        ObjectInfo{&obj, kObjectRef, true, false, true, obj.GetTypeId()}, nullptr
      };

      return *this;
    }

#define MAKE_PROTOTYPE(_Id, _Type)  ObjectPrototype{                     \
      ObjectInfo{ nullptr, kObjectNormal, true, false, true, _Id },      \
      make_shared<_Type>(value)                                          \
    }

    Message &SetObject(bool value) {
      slot_ = MAKE_PROTOTYPE(kTypeIdBool, bool);
      return *this;
    }

    Message &SetObject(int64_t value) {
      slot_ = MAKE_PROTOTYPE(kTypeIdInt, int64_t);
      return *this;
    }

    Message &SetObject(double value) {
      slot_ = MAKE_PROTOTYPE(kTypeIdFloat, double);
      return *this;
    }

    Message &SetObject(string value) {
      slot_ = MAKE_PROTOTYPE(kTypeIdString, string);
      return *this;
    }

#undef MAKE_PROTOTYPE

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
      SetObject(obj);
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
      slot_ = std::nullopt;
      idx_ = 0;
    }
  };

  using CommentedResult = tuple<bool, string>;
}