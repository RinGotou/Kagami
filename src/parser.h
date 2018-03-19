#pragma once
#ifndef _SE_PARSER_
#define _SE_PARSER_

#include "includes.h"

namespace Suzu {
  const string kStrEmpty = "";
  const string kStrFatalError = "__FATAL__";
  const string kStrWarning = "__WARNING__";
  const string kStrSuccess = "__SUCCESS__";
  const string kStrEOF = "__EOF__";
  const string kStrPass = "__PASS__";
  const string kStrNull = "__NULL__";
  const string kStrRedirect = "__*__";
  const string kArgOnce = "@ONCE__";
  const string kstrDefine = "def";
  const string kStrVar = "var";
  const string kStrReturn = "return";
  const string kStrFor = "for";
  const string kStrWhile = "while";
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
  const int kFlagAutoSize = -1;
  const int kFlagNotDefined = -2;
  const size_t kTypeFunction = 0;
  const size_t kTypeString = 1;
  const size_t kTypeInteger = 2;
  const size_t KTypeDouble = 3;
  const size_t kTypeBoolean = 4;
  const size_t kTypeSymbol = 5;
  const size_t kTypeNull = 100;
  const size_t kTypePreserved = 101;
  const regex kPatternFunction(R"([a-zA-Z_][a-zA-Z_0-9]*)");
  const regex kPatternString(R"("(\"|\\|\n|\t|[^"])*")");
  const regex kPatternNumber(R"(\d+\.?\d*)");
  const regex kPatternInteger(R"(\d+)");
  const regex kPatternDouble(R"(\d+\.\d+)");
  const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
  const regex kPatternSymbol(R"(==|<=|>=|&&|\|\||[[:Punct:]])");
  const regex kPatternBlank(R"([[:blank:]])");

  class Message;
  class EntryProvider;
  typedef Message(*Activity)(vector<string> &);

  class StrPair :public pair<string, string> {
  private:
    bool readonly;
  public:
    bool IsReadOnly() const {
      return this->readonly;
    }

    StrPair &SetReadOnly(bool r) {
      this->readonly = r;
    }

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

    string GetValue() const {
      return this->value;
    }

    Message SetCode(const int &code) {
      this->code = code;
      return *this;
    }

    int GetCode() const {
      return this->code;
    }

    Message SetDetail(const string &detail) {
      this->detail = detail;
      return *this;
    }

    string GetDetail() const {
      return this->detail;
    }
  };

  class Util {
  public:
    template <class Type>
    Util CleanUpVector(vector<Type> &target) {
      target.clear();
      vector<Type>(target).swap(target);
      return *this;
    }

    template <class Type>
    Util CleanUpDeque(deque<Type> &target) {
      target.clear();
      deque<Type>(target).swap(target);
      return *this;
    }

    template <class Type>
    bool Compare(Type source, vector<Type> list) {
      bool result = false;
      for (auto unit : list) {
        if (unit == source) {
          result = true;
          break;
        }
      }
      return result;
    }

    template <class Type>
    Type Calc(Type A, Type B, string opercode) {
      Type result = 0;
      if (opercode == "+") result = A + B;
      if (opercode == "-") result = A - B;
      if (opercode == "*") result = A * B;
      if (opercode == "/") result = A / B;
      return result;
    }

    Message GetDataType(string target);
    bool ActivityStart(EntryProvider &provider, vector<string> container,
      deque<string> &item, size_t top, Message &msg);
    Message ScriptStart(string target);
    void PrintEvents();
    void Cleanup();
  };



  class ScriptProvider {
  private:
    std::ifstream stream;
    size_t current;
    vector<string> pool;

    ScriptProvider() {}

    bool IsStreamReady() const {
      return (stream.is_open() && stream.good());
    }
  public:
    ScriptProvider(string target) {
      stream.open(target.c_str(), std::ios::in);
      current = 0;
    }

    ScriptProvider(char *target) {
      stream.open(target, std::ios::in);
      current = 0;
    }

    ~ScriptProvider() {
      stream.close();
      Util().CleanUpVector(pool);
    }

    bool IsPoolReady() const {
      return !(pool.empty());
    }

    size_t WalkBack(size_t step = 1) {
      if (step > current) {
        current = 0;
      }
      else {
        current -= step;
      }

      return current;
    }

    void ResetReader() {
      current = 0;
    }

    bool eof() const {
      return stream.eof();
    }

    void ResetPool() {
      Util().CleanUpVector(pool);
    }

    Message Get();
  };

  class Chainloader {
  private:
    vector<string> raw;
    int GetPriority(string target) const;
  public:
    Chainloader() {}
    Chainloader &Build(vector<string> raw) {
      this->raw = raw;
      return *this;
    }

    Chainloader &Build(string target);
    Chainloader &Reset() {
      Util().CleanUpVector(raw);
      return *this;
    }

    Message Start(); 
  };

  class EntryProvider {
  private:
    string name;
    Activity activity;
    int requiredcount;
    int priority;
  public:
    EntryProvider() : name(kStrNull), activity(nullptr) {
      requiredcount = kFlagNotDefined;
    }
    EntryProvider(string n, Activity a, int r, int p = 1) : name(n) {
      requiredcount = r;
      activity = a;
      priority = p;
    }


    string GetName() const {
      return this->name;
    }

    int GetRequiredCount() const {
      return this->requiredcount;
    }

    int GetPriority() const {
      return this->priority;
    }

    bool Good() const {
      return (activity != nullptr && requiredcount != -2);
    }

    bool operator==(EntryProvider &target) {
      return (target.name == this->name &&
        target.activity == this->activity &&
        target.requiredcount == this->requiredcount);
    }

    Message StartActivity(vector<string> p);
  };

  class MemoryProvider {
  private:
    deque<StrPair> dict;
    MemoryProvider *parent;
    typedef deque<StrPair>::iterator MemPtr;
    StrPair *find(string name);
  public:
    MemoryProvider() {
      parent = nullptr;
    }

    bool empty() const {
      return dict.empty();
    }

    size_t size() const {
      return dict.size();
    }

    void cleanup() {
      Util().CleanUpDeque(dict);
    }

    void create(StrPair unit) {
      if (unit.IsReadOnly()) {
        dict.push_front(unit);
      }
      else {
        dict.push_back(unit);
      }
    }

    MemoryProvider &SetParent(MemoryProvider *ptr) {
      this->parent = ptr;
      return *this;
    }

    MemoryProvider *GetParent() const {
      return parent;
    }

    bool dispose(string name);
    string query(string name);
    string set(string name, string value);
  };

  //TODO:JSON Mini Parser
  //--------------!!WORKING!!-----------------//
  class JSONProvider {
  private:

  public:

  };

  void TotalInjection();

}

namespace Tracking {
  void log(Suzu::Message msg);
}

namespace Entry {
  void Inject(Suzu::EntryProvider provider);
}
#endif // !_SE_PARSER_

