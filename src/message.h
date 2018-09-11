#pragma once
#include "object.h"

namespace kagami {
  class Message {
    string value;
    string detail;
    int code;
    shared_ptr<void> object;
    size_t idx;
  public:
    Message() :
      value(kStrEmpty), detail(kStrEmpty), code(kCodeSuccess), idx(0) {}
    Message(string value, int code, string detail) :
      value(value), detail(detail), code(code), idx(0) {}

    Object GetObj() const;
    void SetObject(Object &object, string id);
    Message &combo(string value, int code, string detail);
    Message &SetValue(const string &value);
    Message &SetCode(const int &code);
    Message &SetDetail(const string &detail);
    Message &SetIndex(const size_t index);

    string GetValue() const { return this->value; }
    int GetCode() const { return this->code; }
    string GetDetail() const { return this->detail; }
    size_t GetIndex() const { return idx; }

  };
}