#pragma once
#include <string>
#include <utility>

namespace Suzu {
  using std::string;
  using std::pair;

  class StrPair :public pair<string, string> {
  private:
    bool readonly;
  public:
    bool IsReadOnly() const { return this->readonly; }
    void SetReadOnly(bool r) { this->readonly = r; }

    StrPair() {
      this->first = "__NULL__";
      this->second = "__NULL__";
    }

    StrPair(string f, string s) {
      this->first = f;
      this->second = s;
    }
  };

  class Message {
  private:
    string value;
    string detail;
    int code;
  public:
    Message() {
      value = ""; //kStrEmpty
      code = 0; //kCodeSuccess
      detail = ""; //kStrEmpty
    }

    Message(string value, int code, string detail) {
      this->value = value;
      this->code = code;
      this->detail = detail;
    }

    Message combo(string value, int code, string detail) {
      this->value = value;
      this->code = code;
      this->detail = detail;
      return *this;
    }

    Message SetValue(const string &value) {
      this->value = value;
      return *this;
    }

    Message SetCode(const int &code) {
      this->code = code;
      return *this;
    }

    Message SetDetail(const string &detail) {
      this->detail = detail;
      return *this;
    }

    string GetValue() const { return this->value; }
    int GetCode() const { return this->code; }
    string GetDetail() const { return this->detail; }
  };
}
