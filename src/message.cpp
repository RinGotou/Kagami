#include "message.h"

namespace kagami {
  void Message::SetObject(Object &object) {
    object_ = make_shared<Object>(object);
    code_ = kCodeObject;
  }

  void Message::SetRawString(string str) {
    value_ = kStrRedirect;
    code_ = kCodeSuccess;
    detail_ = str;
  }

  Object Message::GetObj() const {
    return *static_pointer_cast<Object>(object_);
  }

  Message &Message::SetValue(const string &value) {
    value_ = value;
    return *this;
  }

  Message &Message::SetCode(const int &code) {
    code_ = code;
    return *this;
  }

  Message &Message::SetDetail(const string &detail) {
    detail_ = detail;
    return *this;
  }

  Message &Message::SetIndex(const size_t index) {
    idx_ = index;
    return *this;
  }
}