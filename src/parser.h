#pragma once
#ifndef _SE_PARSER_
#define _SE_PARSER_

#include "includes.h"

namespace Suzu {
  using std::ifstream;
  using std::ofstream;
  using std::vector;
  using std::stack;
  using std::array;
  using std::deque;
  using std::regex;
  using std::pair;
  using std::to_string;
  using std::stoi;
  using std::stof;
  using std::stod;
  using std::regex_match;

  const regex kPatternFunction(R"([a-zA-Z_][a-zA-Z_0-9]*)");
  const regex kPatternString(R"("(\"|\\|\n|\t|[^"]|[[:Punct:]])*")");
  const regex kPatternNumber(R"(\d+\.?\d*)");
  const regex kPatternInteger(R"(\d+)");
  const regex kPatternDouble(R"(\d+\.\d+)");
  const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
  const regex kPatternSymbol(R"(==|<=|>=|&&|\|\||[[:Punct:]])");
  const regex kPatternBlank(R"([[:blank:]])");

  class EntryProvider;
  typedef Message(*Activity)(vector<string> &);
  typedef Message *(*PluginActivity)(vector<string> &);

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
    bool cached;
    ScriptProvider() {}

    bool IsStreamReady() const { return stream.good(); }
  public:
    ScriptProvider(string target) {
      stream.open(target.c_str(), std::ios::in);
      cached = false;
      current = 0;
    }

    ScriptProvider(char *target) {
      stream.open(target, std::ios::in);
      cached = false;
      current = 0;
    }

    ~ScriptProvider() {
      stream.close();
      Util().CleanUpVector(pool);
    }

    size_t WalkBack(size_t step = 1) {
      if (step > current) current = 0;
      else current -= step;
      return current;
    }

    bool IsPoolReady() const { return !(pool.empty()); }
    void ResetReader() { current = 0; }
    void ResetPool() { Util().CleanUpVector(pool); }
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
  protected:
    string name;
    Activity activity;
    PluginActivity activity2;
    int requiredcount;
    int priority;
    //bool needempty;
  public:
    EntryProvider() : name(kStrNull), activity(nullptr) {
      requiredcount = kFlagNotDefined;
    }

    EntryProvider(string n, Activity a, int r, int p = kFlagNormalEntry) : name(n) {
      requiredcount = r;
      activity = a;
      priority = p;
      activity2 = nullptr;
    }

    EntryProvider(string n,PluginActivity p) : name(n) {
      priority = kFlagPluginEntry;
      requiredcount = kFlagAutoSize;
      activity2 = p;
      activity = nullptr;
    }

    bool operator==(EntryProvider &target) {
      return (target.name == this->name &&
        target.activity == this->activity &&
        target.requiredcount == this->requiredcount);
    }

    string GetName() const { return this->name; }
    int GetRequiredCount() const { return this->requiredcount; }
    int GetPriority() const { return this->priority; }
    bool Good() const { return (activity != nullptr && requiredcount != -2); }
    Message StartActivity(vector<string> p);
  };

  //TODO:JSON Mini Parser
  //--------------!!WORKING!!-----------------//
  //class JSONProvider {
  //private:
  //  vector<StrPair> base;
  //  JSONProvider *childbase;
  //public:
  //  JSONProvider(string path) {

  //  }

  //  string GetValue(string key) {

  //  }

  //  string GetChild(string key) {

  //  }
  //};

  void TotalInjection();
}

namespace Tracking {
  using Suzu::Message;
  using std::vector;
  void log(Suzu::Message msg);
}

namespace Entry {
  using namespace Suzu;
  void Inject(Suzu::EntryProvider provider);
  void Delete(std::string name);
  void ResetPluginEntry();
}
#endif // !_SE_PARSER_

