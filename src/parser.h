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
#include "includes.h"

namespace Kagami {
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
  const string kCastNull = "null";
  const string kCastInt = "int";
  const string kCastString = "string";
  const string kCastDeque = "deque";

  const regex kPatternFunction(R"([a-zA-Z_][a-zA-Z_0-9]*)");
  const regex kPatternNumber(R"(\d+\.?\d*)");
  const regex kPatternInteger(R"([-]?\d+)");
  const regex kPatternDouble(R"([-]?\d+\.\d+)");
  const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
  const regex kPatternSymbol(R"(==|<=|>=|!=|&&|\|\||[[:Punct:]])");
  const regex kPatternBlank(R"([[:blank:]])");

  class EntryProvider;
  class Chainloader;

  /*Kit Class
   this class contains many useful template or tiny function, and
   create script processing workspace.
  */
  class Kit {
  public:
    template <class Type>
    Kit CleanupVector(vector<Type> &target) {
      target.clear();
      vector<Type>(target).swap(target);
      return *this;
    }

    template <class Type>
    Kit CleanupDeque(deque<Type> &target) {
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

    Message ExecScriptFile(string target);
    void PrintEvents();
    void Terminal();
    vector<string> BuildStringVector(string source);
  };

  /*PointWrapper Class
   A shared void pointer is packaged in this.Almost all varibales and
   constants are managed by shared pointers.This class will be packaged
   in MemoryManager class.
  */
  class PointWrapper {
  private:
    std::shared_ptr<void> ptr;
    string castoption;
  public:
    PointWrapper() { ptr = nullptr; castoption = kCastNull; }
    template <class T> PointWrapper &manage(T &t, string castoption) {
      ptr = std::make_shared<T>(t);
      this->castoption = castoption;
      return *this;
    }
    PointWrapper &set(shared_ptr<void> ptr, string castoption) {
      this->ptr = ptr;
      this->castoption = castoption;
      return *this;
    }
    shared_ptr<void> get() { return ptr; }
    string getOption() const { return castoption; }
  };

  /*MemoryManager Class
  MemoryManger will be filled with PointWrapper and manage life cycle of variables
  and constants.
  */
  class MemoryManager {
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
    template <class T> void CreateByObject(string name, T &t, string castoption, bool ro) {
      auto insert = [&]() {
        base.insert(PointBase::value_type(name, PointWrapper().manage(t, castoption)));
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

    PointWrapper *Find(string name) {
      PointWrapper *wrapper = nullptr;
      for (auto &unit : base) {
        if (unit.first == name) {
          wrapper = &(unit.second);
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

  /*ScriptProvider class
  Script provider caches original string data from script file.
  */
  class ScriptProvider {
  private:
    std::ifstream stream;
    size_t current;
    vector<string> base;
    bool health;
    bool end;
    ScriptProvider() {}
  public:
    ~ScriptProvider() {
      stream.close();
      Kit().CleanupVector(base);
    }

    bool GetHealth() const { return health; }
    bool eof() const { return end; }
    void ResetCounter() { current = 0; }
    ScriptProvider(const char *target);
    Message Get();
  };

  /*Chainloader Class
  The most important part of script processor.Original string will be tokenized and
  parsed here.Processed data will be delivered to entry provider.
  */
  class Chainloader {
  private:
    vector<string> raw;
    map<string, PointWrapper> lambdamap;
    int GetPriority(string target) const;
    bool StartActivity(EntryProvider &provider, deque<string> container,
      deque<string> &item, size_t top, Message &msg, Chainloader *loader);
    bool ShuntingYardProcessing(bool disableset, deque<string> &item, deque<string> &symbol, 
      Message &msg, size_t mode);
  public:
    Chainloader() {}
    Chainloader &Build(vector<string> raw) {
      this->raw = raw;
      return *this;
    }

    PointWrapper GetVariable(string name) {
      PointWrapper result;
      map<string, PointWrapper>::iterator it = lambdamap.find(name);
      if (it != lambdamap.end()) result = it->second;
      return result;
    }

    Chainloader &Reset() {
      Kit().CleanupVector(raw);
      return *this;
    }

    Message Start(size_t mode); 
    Chainloader &Build(string target);
  };

  /*ChainStorage Class
  A set of packaged chainloader.Loop and condition judging is processed here.
  */
  class ChainStorage {
  private:
    vector<Chainloader> storage;
    vector<string> parameter;
    void AddLoader(string raw) {
      storage.push_back(Chainloader().Reset().Build(raw));
    }
  public:
    ChainStorage(ScriptProvider &provider) {
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
  
  /*EntryProvider Class
  contains function pointer.Processed argument tokens are used for building
  new argument map.Entry provider have two mode:internal function and plugin
  function.
  */
  class EntryProvider {
  private:
    string name;
    int requiredcount;
    int priority;
    vector<string> parameters;
    Activity activity;
    PluginActivity activity2;
    MemoryDeleter deleter;
  public:
    EntryProvider() : name(kStrNull), activity(nullptr), activity2(nullptr), deleter(nullptr) {
      requiredcount = kFlagNotDefined;
    }

    EntryProvider(string n, Activity a, int r, int p = kFlagNormalEntry, vector<string> pa = vector<string>()) :
    name(n), parameters(pa), activity(a), activity2(nullptr), deleter(nullptr) {
      requiredcount = r;
      priority = p;
    }

    EntryProvider(string n, PluginActivity p, vector<string> pa, MemoryDeleter d) : 
      name(n), parameters(pa), activity(nullptr), activity2(p), deleter(d) {
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

  inline string CastToString(shared_ptr<void> ptr) {
    return *static_pointer_cast<string>(ptr);
  }

  void InjectBasicEntries();
}

/*stuff of event tracking*/
namespace Tracking {
  using Kagami::Message;
  using std::vector;
  void log(Kagami::Message msg);
}

/*stuff of entry storage,plugin instance managing and etc.*/
namespace Entry {
  using namespace Kagami;
  typedef map<string, EntryProvider> EntryMap;
  typedef map<string, EntryProvider>::value_type EntryMapUnit;
  extern vector<MemoryManager> MemoryAdapter;

  void Inject(string name, EntryProvider provider);
  void Delete(string name);
  void ResetPluginEntry();
  void ResetPlugin(bool OnExit = false);
  void DisposeWrapper(string name, bool reserved);
  void CleanupWrapper();
  PointWrapper *FindWrapper(string name, bool reserved);
  MemoryManager CreateMap();
  bool DisposeMap();

  template <class T>
  PointWrapper *CreateWrapper(string name, T t, string castoption, bool readonly = false) {
    PointWrapper *wrapper = nullptr;
    if (Kit().GetDataType(name) != kTypeFunction) {
      Tracking::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name"));
      return nullptr;
    }
    MemoryAdapter.back().CreateByObject(name, t, castoption, false);
    wrapper = MemoryAdapter.back().Find(name);
    return wrapper;
  }
}
