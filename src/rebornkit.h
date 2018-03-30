#pragma once
#include <string>
#include <utility>
#include <cstring>

#define _CRT_SECURE_NO_WARNINGS

namespace Suzu {
  using std::string;
  using std::pair;

  const string kEngineVersion = "version 0.1 'anzu'";
  const string kEngineName = "RebornScripter";
  const string kEngineAuthor = "Suzu Nakamura";
  const string kCopyright = "(C) 2017-2018";
  const string kStrEmpty = "";
  const string kStrFatalError = "__FATAL__";
  const string kStrWarning = "__WARNING__";
  const string kStrSuccess = "__SUCCESS__";
  const string kStrEOF = "__EOF__";
  const string kStrPass = "__PASS__";
  const string kStrNull = "__NULL__";
  const string kStrRedirect = "__*__";
  const string kStrTrue = "true";
  const string kStrFalse = "false";
  const int kCodeQuit = 3;
  const int kCodeRedirect = 2;
  const int kCodeNothing = 1;
  const int kCodeSuccess = 0;
  const int kCodeBrokenEntry = -1;
  const int kCodeOverflow = -2;
  const int kCodeIllegalArgs = -3;
  const int kCodeIllegalCall = -4;
  const int kCodeIllegalSymbol = -5;
  const int kCodeBadStream = -6;
  const int kFlagCoreEntry = 0;
  const int kFlagNormalEntry = 1;
  const int kFlagBinEntry = 2;
  const int kFlagPluginEntry = 3;
  const int kFlagAutoSize = -1;
  const int kFlagNotDefined = -2;
  const size_t kTypeFunction = 0;
  const size_t kTypeString = 1;
  const size_t kTypeInteger = 2;
  const size_t KTypeDouble = 3;
  const size_t kTypeBoolean = 4;
  const size_t kTypeSymbol = 5;
  const size_t kTypeBlank = 6;
  const size_t kTypeNull = 100;
  const size_t kTypePreserved = 101;

  typedef struct {
    char *value;
    int code;
    char *detail;
  }MsgValue;


  class StrPair :public pair<string, string> {
  private:
    bool readonly;
  public:
    bool IsReadOnly() const { return this->readonly; }
    void SetReadOnly(bool r) { this->readonly = r; }

    StrPair() {
      this->first = kStrNull;
      this->second = kStrNull;
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
      value = kStrEmpty; 
      code = kCodeSuccess;
      detail = kStrEmpty;
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
