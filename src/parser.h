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
#pragma execution_character_set("utf-8")
#define _ENABLE_FASTRING_
#include <stack>
#include <fstream>
#include <array>
#include "kagamicommon.h"
#if defined(_WIN32)
#include "windows.h"
#define WIN32_LEAN_AND_MEAN
#else
#endif

namespace kagami {
  using std::ifstream;
  using std::ofstream;
  using std::stack;
  using std::array;
  using std::to_string;
  using std::stoi;
  using std::stof;
  using std::stod;

  const string kStrNop = "nop";
  const string kStrDef = "def";

  /*ObjectManager Class
  MemoryManger will be filled with Object and manage life cycle of variables
  and constants.
  */
  class ObjectManager {
  private:
    using ObjectBase = map<string, Object>;
    ObjectBase base;

    bool CheckObject(string sign) {
      const auto it = base.find(sign);
      if (it == base.end()) return false;
      else return true;
    }
  public:
    template <class Type> 
    bool Create(string sign, Type &t, string TypeId, ObjTemplate temp, bool constant) {
      auto result = true;
      //string tag;
      Attribute attribute;

      if (CheckObject(sign) == true) {
        result = false;
      }
      else {
        if (constant) attribute.ro = true;
        else attribute.ro = false;
        attribute.methods = temp.GetMethods();

        auto tag = Kit().BuildAttrStr(attribute);

        base.insert(pair<string, Object>(sign, Object().Manage(t, TypeId, tag)));
      }
      
      return result;
    }
    bool Add(string sign, Object &source) {
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

    void Dispose(string name) {
      const auto it = base.find(name);
      if (it != base.end()) base.erase(it);
    }
  };

  /*Processor Class
  The most important part of script processor.Original string will be tokenized and
  parsed here.Processed data will be delivered to entry provider.
  */
  class Processor {
  private:
    bool health;
    //bool legal;
    vector<string> raw;
    map<string, Object> lambdamap;
    deque<string> item, symbol;
    bool commaExpFunc,
      insertBtnSymbols,
      disableSetEntry,
      dotOperator,
      defineLine,
      functionLine,
      subscriptProcessing;
    string currentToken;
    string nextToken;
    string forwardToken;
    string operatorTargetType;
    string errorString;
    size_t mode,
      nextInsertSubscript,
      lambdaObjectCount;

    void EqualMark();
    void Comma();
    bool LeftBracket(Message &msg);
    bool RightBracket(Message &msg);
    void LeftSquareBracket();
    bool RightSquareBracket(Message &msg);
    void Dot();
    void OtherSymbols();
    bool FunctionAndObject(Message &msg);
    void OtherTokens();
    void FinalProcessing(Message &msg);
    Object *GetObj(string name);
    static vector<string> Spilt(string target);
    static string GetHead(string target);
    int GetPriority(string target) const;
    bool Assemble(Message &msg);
  public:
    Processor(): health(false), commaExpFunc(false), insertBtnSymbols(false), disableSetEntry(false),
                 dotOperator(false), defineLine(false), functionLine(false), subscriptProcessing(false), mode(0),
                 nextInsertSubscript(0), lambdaObjectCount(0) {}

    Processor &Build(vector<string> &raw) {
      this->raw = raw;
      return *this;
    }

    Processor &Reset() {
      Kit().CleanupVector(raw);
      return *this;
    }

    Message Start(size_t mode = kModeNormal);
    Processor &Build(string target);
    bool IsHealth() const { return health; }
    string GetErrorString() const { return errorString; }
  };

  /*ScriptProvider class
  Script provider caches original string data from script file.
  */
  class ScriptProvider {
  private:
    std::ifstream stream;
    size_t current;
    //vector<string> base;
    vector<Processor> storage;
    vector<string> parameters;
    //string errorString;
    bool health;
    bool end;


    // ReSharper disable CppPossiblyUninitializedMember
    ScriptProvider() {}
    // ReSharper restore CppPossiblyUninitializedMember
    void AddLoader(string raw) { 
      storage.push_back(Processor().Reset().Build(raw)); 
    }

    static bool IsBlankStr(string target) {
      if (target == kStrEmpty || target.size() == 0) return true;
      for (const auto unit : target) {
        if (unit != ' ' || unit != '\n' || unit != '\t' || unit != '\r') {
          return false;
        }
      }
      return true;
    }
  public:
    ~ScriptProvider() {
      stream.close();
      Kit().CleanupVector(storage).CleanupVector(parameters);
    }

    bool GetHealth() const { return health; }
    bool Eof() const { return end; }
    void ResetCounter() { current = 0; }
    explicit ScriptProvider(const char *target);
    Message Run(deque<string> res = deque<string>());
  };

  /*EntryProvider Class
  contains function pointer.Processed argument tokens are used for building
  new argument map.entry provider have two mode:internal function and plugin
  function.
  */
  class EntryProvider {
  private:
    string id;
    int argMode;
    int priority;
    vector<string> args;
    Activity activity;
    string specifictype;
    size_t minsize;
  public:
    EntryProvider() : id(kStrNull), priority(0), activity(nullptr), minsize(0) {
      argMode = kCodeIllegalArgs;
      specifictype = kTypeIdNull;
    }

    explicit EntryProvider(ActivityTemplate temp) :
      id(temp.id), args(Kit().BuildStringVector(temp.args)),
      activity(temp.activity), minsize(0) {
      argMode = temp.argMode;
      priority = temp.priority;
      specifictype = temp.specifictype;
    }

    bool operator==(EntryProvider &target) const {
      return (target.id == this->id &&
        target.activity == this->activity &&
        target.argMode == this->argMode &&
        target.priority == this->priority &&
        this->specifictype == target.specifictype &&
        target.args == this->args);
    }

    bool operator==(ActivityTemplate &target) const {
      return(
        this->id == target.id &&
        this->argMode == target.argMode &&
        this->priority==target.priority &&
        this->args == Kit().BuildStringVector(target.args) &&
        this->activity == target.activity &&
        this->specifictype == target.specifictype
        );
    }

    EntryProvider &SetSpecificType(string type) {
      this->specifictype = type;
      return *this;
    }

    string GetSpecificType() const { return specifictype; }
    string GetId() const { return this->id; }
    int GetArgumentMode() const { return this->argMode; }
    vector<string> GetArguments() const { return args; }
    size_t GetArgumentSize() const { return this->args.size(); }
    int GetPriority() const { return this->priority; }
    bool Good() const { return ((activity != nullptr) && argMode != kCodeIllegalArgs); }
    Message Start(ObjectMap &map) const;
  };

  inline string CastToString(shared_ptr<void> ptr) {
    return *static_pointer_cast<string>(ptr);
  }

  void Activiate();
  void InitTemplates();
  void InitMethods();

  namespace type {
    ObjTemplate *GetTemplate(string name);
    void AddTemplate(string name, ObjTemplate temp);
    shared_ptr<void> GetObjectCopy(Object &object);
  }

  namespace trace {
    using log_t = pair<string, Message>;
    void Log(kagami::Message msg);
    vector<log_t> &GetLogger();
  }

  namespace entry {
#if defined(_WIN32)
    //Windows Verison
    class Instance : public pair<string, HINSTANCE> {
    private:
      bool health;
      vector<ActivityTemplate> actTemp;
    public:
      Instance() { health = false; }
      bool Load(string name, HINSTANCE h);
      bool GetHealth() const { return health; }
      vector<ActivityTemplate> GetMap() const { return actTemp; }
      CastAttachment GetObjTemplate() const { return CastAttachment(GetProcAddress(this->second, "CastAttachment")); }
      MemoryDeleter GetDeleter() const { return MemoryDeleter(GetProcAddress(this->second, "FreeMemory")); }
    };
#else
    //Linux Version
#endif
    using EntryMapUnit = map<string, EntryProvider>::value_type;
    
    string GetTypeId(string sign);
    void Inject(EntryProvider provider);
    void RemoveByTemplate(ActivityTemplate temp);
    Object *FindObject(string name);
    ObjectManager &CreateManager();
    bool DisposeManager();
    size_t ResetPlugin();
    EntryProvider Order(string id, string type, int size);
    std::wstring s2ws(const std::string& s);
  }
}


