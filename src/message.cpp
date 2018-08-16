#include "message.h"

namespace kagami {
  void Message::SetObject(Object &object, string id) {
    this->object = make_shared<Object>(object);
    this->code = kCodeObject;
    this->detail = id;
  }

  Object Message::GetObj() const {
    return *static_pointer_cast<Object>(this->object);
  }

  Message &Message::combo(string value, int code, string detail) {
    this->value = value;
    this->code = code;
    this->detail = detail;
    return *this;
  }

  Message &Message::SetValue(const string &value) {
    this->value = value;
    return *this;
  }

  Message &Message::SetCode(const int &code) {
    this->code = code;
    return *this;
  }

  Message &Message::SetDetail(const string &detail) {
    this->detail = detail;
    return *this;
  }

  Message &Message::SetIndex(const size_t index) {
    idx = index;
    return *this;
  }
}