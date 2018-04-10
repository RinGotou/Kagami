//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Suzu Nakamura
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//*Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
  using std::map;
  using std::to_string;
  using std::stoi;
  using std::stof;
  using std::stod;
  using std::regex_match;
  using std::shared_ptr;
  using std::static_pointer_cast;
  using std::make_shared;
  const string kStrVar = "var";
  const string kStrFor = "for";
  const string kStrForeach = "foreach";
  const string kStrWhile = "while";
  const string kStrEnd = "end";
  const regex kPatternFunction(R"([a-zA-Z_][a-zA-Z_0-9]*)");
  //const regex kPatternString(R"("(\"|\\|\n|\t|[^"]|[[:Punct:]])*")");
  const regex kPatternNumber(R"(\d+\.?\d*)");
  const regex kPatternInteger(R"([-]?\d+)");
  const regex kPatternDouble(R"([-]?\d+\.\d+)");
  const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
  const regex kPatternSymbol(R"(==|<=|>=|&&|\|\||[[:Punct:]])");
  const regex kPatternBlank(R"([[:blank:]])");

  class EntryProvider;
  class Chainloader;
  typedef map<string, string> StrMap;
  typedef map<string, shared_ptr<void>> PathMap;
  typedef Message(*Activity)(PathMap &);
  typedef Message *(*PluginActivity)(PathMap &);

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

    template <class Type>
    bool Logic(Type A, Type B, string opercode) {
      bool result = false;
      if (opercode == "==") result = (A == B);
      if (opercode == "<=") result = (A <= B);
      if (opercode == ">=") result = (A >= B);
      if (opercode == "!=") result = (A != B);
      return result;
    }

    string GetRawString(string target) {
      return target.substr(1, target.size() - 2);
    }

    int GetDataType(string target);

    Message ScriptStart(string target);
    void PrintEvents();
    //void Cleanup();
    void Terminal();
    vector<string> BuildStrVec(string source);
  };

  class ScriptProvider2 {
  private:
    std::ifstream stream;
    size_t current;
    vector<string> base;
    bool health;
    bool end;
    ScriptProvider2() {}
  public:
    ~ScriptProvider2() {
      stream.close();
      Util().CleanUpVector(base);
    }

    bool GetHealth() const { return health; }
    bool eof() const { return end; }
    void ResetCounter() { current = 0; }
    ScriptProvider2(const char *target);
    Message Get();
  };

  class Chainloader {
  private:
    vector<string> raw;
    map<string, shared_ptr<void>> lambdamap;
    int GetPriority(string target) const;
    bool ActivityStart(EntryProvider &provider, deque<string> container,
      deque<string> &item, size_t top, Message &msg, Chainloader *loader);
    bool StartCode(bool disableset, deque<string> &item, deque<string> &symbol, 
      Message &msg);
  public:
    Chainloader() {}
    Chainloader &Build(vector<string> raw) {
      this->raw = raw;
      return *this;
    }

    Chainloader &Build(string target);
    shared_ptr<void> GetVariable(string name) {
      shared_ptr<void> result;
      map<string, shared_ptr<void>>::iterator it = lambdamap.find(name);
      if (it != lambdamap.end()) result = it->second;
      return result;
    }
    Chainloader &Reset() {
      Util().CleanUpVector(raw);
      return *this;
    }

    Message Start(); 
  };

  class ChainStorage {
  private:
    vector<Chainloader> storage;
    vector<string> parameter;
    void AddLoader(string raw) {
      storage.push_back(Chainloader().Reset().Build(raw));
    }
  public:
    ChainStorage(ScriptProvider2 &provider) {
      Message temp;
      while (provider.eof() != true) {
        temp = provider.Get();
        if (temp.GetCode() == kCodeSuccess) AddLoader(temp.GetDetail());
      }
    }

    void AddParameterName(vector<string> names) {
      this->parameter = names;
    }

    Message Run(deque<string> res);
  };

  class EntryProvider {
  protected:
    string name;
    Activity activity;
    PluginActivity activity2;
    int requiredcount;
    int priority;
    vector<string> parameters;
  public:
    EntryProvider() : name(kStrNull), activity(nullptr), activity2(nullptr) {
      requiredcount = kFlagNotDefined;
    }

    EntryProvider(string n, Activity a, int r, int p = kFlagNormalEntry, vector<string> pa = vector<string>()) :
    name(n), parameters(pa), activity(a), activity2(nullptr) {
      requiredcount = r;
      priority = p;
    }

    EntryProvider(string n, PluginActivity p, vector<string> pa): 
      name(n), parameters(pa), activity(nullptr), activity2(p) {
      priority = kFlagPluginEntry;
      requiredcount = kFlagAutoFill;
    }

    bool operator==(EntryProvider &target) {
      return (target.name == this->name &&
        target.activity == this->activity &&
        target.requiredcount == this->requiredcount &&
        target.activity2 == this->activity2 &&
        target.priority == this->priority &&
        target.parameters == this->parameters);
    }

    string GetName() const { return this->name; }
    int GetRequiredCount() const { return this->requiredcount; }
    int GetPriority() const { return this->priority; }
    bool Good() const { return ((activity != nullptr || activity2 != nullptr) && requiredcount != kFlagNotDefined); }
    Message StartActivity(deque<string> p, Chainloader *parent);
  };

  class PointWrapper {
  private:
    std::shared_ptr<void> ptr;
  public:
    PointWrapper() { ptr = nullptr; }
    template <class T> PointWrapper(T &t) { ptr = std::make_shared<T>(t); }
    void set(shared_ptr<void> ptr) { this->ptr = ptr; }
    shared_ptr<void> get() { return ptr; }
  };

  class PointMap {
  private:
    typedef map<string, PointWrapper> PointBase;
    PointBase base;
    vector<string> rolist;

    bool CheckPriority(string name) {
      if (rolist.empty()) {
        return true;
      }
      else {
        for (auto unit : rolist) {
          if (unit == name) return true;
        }
      }
      return false;
    }
  public:
    template <class T> void CreateByObject(string name, T &t, bool ro) {
      auto insert = [&]() {
        base.insert(PointBase::value_type(name, PointWrapper(t)));
        if (ro) rolist.emplace_back(name);
      };
      switch (base.empty()) {
      case true:insert(); break;
      case false:
        PointBase::iterator i = base.find(name);
        if (i == base.end()) insert();
      }
    }

    void CreateByWrapper(string name, PointWrapper source, bool ro) {
      auto insert = [&]() {
        base.insert(PointBase::value_type(name, source));
        if (ro) rolist.emplace_back(name);
      };
      switch (base.empty()) {
      case true:insert(); break;
      case false:
        PointBase::iterator i = base.find(name);
        if (i == base.end()) insert();
      }
    }

    PointWrapper Find(string name) {
      PointWrapper wrapper;
      for (auto unit : base) {
        if (unit.first == name) {
          wrapper = unit.second;
          break;
        }
      }
      return wrapper;
    }

    void dispose(string name) {
      PointBase::iterator it = base.find(name);
      if (it != base.end()) base.erase(it);
    }
  };

  inline string CastToString(shared_ptr<void> ptr) {
    return *static_pointer_cast<string>(ptr);
  }

  void TotalInjection();
}

namespace Tracking {
  using Suzu::Message;
  using std::vector;
  void log(Suzu::Message msg);
}

namespace Entry {
  using namespace Suzu;
  typedef map<string, EntryProvider> EntryMap;
  typedef map<string, EntryProvider>::value_type EntryMapUnit;
  extern vector<PointMap> MemoryAdapter;

  void Inject(string name, EntryProvider provider);
  void Delete(string name);
  void ResetPluginEntry();
  void ResetPlugin(bool OnExit = false);
  void DisposeWrapper(string name, bool reserved);
  void CleanupWrapper();
  PointWrapper FindWrapper(string name, bool reserved);
  PointMap CreateMap();
  bool DisposeMap();

  template <class T>
  PointWrapper CreateWrapper(string name, T t, bool readonly = false) {
    PointWrapper wrapper;
    if (Util().GetDataType(name) != kTypeFunction) {
      Tracking::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name"));
      return wrapper;
    }
    MemoryAdapter.back().CreateByObject(name, t, false);
    wrapper = MemoryAdapter.back().Find(name);
    return wrapper;
  }
}
#endif // !_SE_PARSER_

