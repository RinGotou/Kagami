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

namespace kagami {
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
  const string kTypeIdNull = "null";
  const string kTypeIdInt = "int";
  const string kTypeIdRawString = "string";
  const string kTypeIdArrayBase = "deque";
  const string kTypeIdRef = "__ref";

  const regex kPatternFunction(R"([a-zA-Z_][a-zA-Z_0-9]*)");
  const regex kPatternNumber(R"(\d+\.?\d*)");
  const regex kPatternInteger(R"([-]?\d+)");
  const regex kPatternDouble(R"([-]?\d+\.\d+)");
  const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
  const regex kPatternSymbol(R"(==|<=|>=|!=|&&|\|\||[[:Punct:]])");
  const regex kPatternBlank(R"([[:blank:]])");

  class EntryProvider;
  class Chainloader;

#if defined(_ENABLE_FASTRING_)
  /*Object Tag Struct
    no description yet.
  */
  struct AttrTag {
    string methods;
    bool ro;
    AttrTag(string methods, bool ro) {
      this->methods = methods;
      this->ro = ro;
    }
    AttrTag(){}
  };
#endif
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
    AttrTag GetAttrTag(string target);
    string MakeAttrTagStr(AttrTag target);
    Message ExecScriptFile(string target);
    void PrintEvents();
    void Terminal();
    vector<string> BuildStringVector(string source);
  };



  /*Object Class
   A shared void pointer is packaged in this.Almost all varibales and
   constants are managed by shared pointers.This class will be packaged
   in ObjectManager class.
  */
  class Object {
  private:
    std::shared_ptr<void> ptr;
    string option;
    string tag;
  public:
    Object() { ptr = nullptr; option = kTypeIdNull; }
    template <class T> Object &manage(T &t, string option, string tag) {
      ptr = std::make_shared<T>(t);
      this->option = option;
      this->tag = tag;
      return *this;
    }
    Object &set(shared_ptr<void> ptr, string option,string tag) {
      this->ptr = ptr;
      this->option = option;
      return *this;
    }
    shared_ptr<void> get() { return ptr; }
    string GetTypeId() const { return option; }
    AttrTag getTag() const { return Kit().GetAttrTag(tag); }
    Object &setTag(string tag) { this->tag = tag; return *this; }
    AttrTag addTag(string target) { tag.append(target); return Kit().GetAttrTag(tag); }
  };

  /*ObjectManager Class
  MemoryManger will be filled with Object and manage life cycle of variables
  and constants.
  */
  class ObjectManager {
  private:
    using ObjectBase = map<string, Object>;
    ObjectBase base;

    bool CheckObject(string sign) {
      ObjectBase::iterator it = base.find(sign);
      if (it == base.end()) return false;
      else return true;
    }
  public:
    template <class Type> bool Create(string sign, Type &t, string TypeId, ObjTemplate temp, bool constant) {
      bool result = true;
      string tag = kStrEmpty;
      AttrTag attrTag;

      if (CheckObject(sign) == true) {
        result = false;
      }
      else {
        if (constant) attrTag.ro = true;
        else attrTag.ro = false;
        attrTag.methods = temp.GetMethods();

        tag = Kit().MakeAttrTagStr(attrTag);

        base.insert(pair<string, Object>(sign, Object().manage(t, TypeId, tag)));
      }
      
      return result;
    }
    bool add(string sign, Object &source) {
      bool result = true;
      Object object = source;

      if (CheckObject(sign) == true) {
        result = false;
      }
      else {
        base.insert(pair<string, Object>(sign, object));
      }
      return result;
    }

    Object *Find(string name) {
      Object *wrapper = nullptr;
      for (auto &unit : base) {
        if (unit.first == name) {
          wrapper = &(unit.second);
          break;
        }
      }
      return wrapper;
    }

    void dispose(string name) {
      ObjectBase::iterator it = base.find(name);
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
    map<string, Object> lambdamap;
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

    Object GetVariable(string name) {
      Object result;
      map<string, Object>::iterator it = lambdamap.find(name);
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
  new argument map.entry provider have two mode:internal function and plugin
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

  void Activiate();

  namespace type {
    ObjTemplate *GetTemplate(string name);
  }

  namespace trace {
    using log_t = pair<string, Message>;
    void log(kagami::Message msg);
  }

  namespace entry {
    extern vector<ObjectManager> ObjectStack;

    using EntryMap = map<string, EntryProvider>;
    using EntryMapUnit = map<string, EntryProvider>::value_type;

#if defined(_WIN32)
    //Windows Verison
    class Instance : public pair<string, HINSTANCE> {
    private:
      bool health;
      StrMap link_map;
    public:
      Instance() { health = false; }
      bool Load(string name, HINSTANCE h);
      bool GetHealth() const { return health; }
      StrMap GetMap() const { return link_map; }
      CastAttachment getObjTemplate() { return (CastAttachment)GetProcAddress(this->second, "CastAttachment"); }
      MemoryDeleter getDeleter() { return (MemoryDeleter)GetProcAddress(this->second, "FreeMemory"); }
    };
#else
    //Linux Version
#endif
    
    string GetTypeId(string sign);
    std::wstring s2ws(const std::string& s);
    void Inject(string name, EntryProvider provider);
    void Delete(string name);
    void ResetPluginEntry();
    void ResetPlugin(bool OnExit = false);
    Object *FindObject(string name);
    ObjectManager &CreateManager();
    bool DisposeManager();

    //template <class T>
    //Object *CreateObject(string name, T t, string option, bool readonly = false) {
    //  Object *object = nullptr;
    //  if (Kit().GetDataType(name) != kTypeFunction) {
    //    trace::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name"));
    //    return nullptr;
    //  }
    //  ObjectStack.back().CreateByObject(name, t, option, false);
    //  object = ObjectStack.back().Find(name);
    //  return object;
    //}
  }
}


