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
#include "akane/akane.h"
#include <stack>
#include <fstream>
#include "kagamicommon.h"
#if defined(_WIN32)
#include "windows.h"
#define WIN32_LEAN_AND_MEAN
#else
#endif
//#define _NO_CUI_

namespace kagami {
  using std::ifstream;
  using std::ofstream;
  using std::stack;
  using std::to_string;
  using std::stoi;
  using std::stof;
  using std::stod;
  using akane::list;

  const string kStrNop = "nop";
  const string kStrDef = "def";
  const string kStrRef = "__ref";
  const string kStrCodeSub = "__code_sub";
  const string kStrSub = "__sub";

  /*ObjectManager Class
  MemoryManger will be filled with Object and manage life cycle of variables
  and constants.
  */
  class ObjectManager {
    using NamedObject = pair<string,Object>;
    list<NamedObject> base;

    bool CheckObject(string sign) {
      //NamedObject *object = nullptr;
      for (size_t i = 0;i < base.size();++i) {
        NamedObject &object = base.at(i);
        if (object.first == sign) return false;
      }
      return true;
    }
  public:
    ObjectManager() {}
    ObjectManager(ObjectManager &mgr) { base = mgr.base; }
    ObjectManager(ObjectManager &&mgr) {}
    ObjectManager &operator=(ObjectManager &mgr) {
      base = mgr.base;
      return *this;
    } 
    bool Add(string sign, Object source) {
      if(!CheckObject(sign)) return false;
      base.push_back(NamedObject(sign,source));
      return true;
    }
    Object *Find(string sign) {
      Object *object = nullptr;
      for (size_t i = 0;i < base.size();++i) {
        if (base[i].first == sign) {
          object = &(base[i].second);
          break;
        }
      }
      return object;
    }
    void Dispose(string sign) {
      size_t pos = 0;
      bool found = false;
      for (size_t i = 0;i < base.size();++i) {
        if (base[i].first == sign) {
          found = true;
          pos = i;
          break;
        }
      }
      if (found) base.erase(pos);
    }
    void clear() {
      list<NamedObject> temp;
      for(size_t i = 0;i < base.size();++i) {
        if(base[i].second.IsPermanent()) {
          temp.push_back(base[i]);
        }
      }
      base.clear();
      base = temp;
    }
    bool Empty() const {
      return base.empty();
    }
  };

  /*Processor Class
  The most important part of script processor.Original string will be tokenized and
  parsed here.Processed data will be delivered to entry provider.
  */
  class Processor {
    using Token = pair<string, size_t>;
    bool health;
    vector<Token> origin;
    vector<size_t> types;
    map<string, Object> lambdamap;
    deque<Token> item, symbol;
    bool commaExpFunc,
      insertBtnSymbols,
      disableSetEntry,
      dotOperator,
      defineLine,
      functionLine,
      subscriptProcessing;
    Token currentToken;
    Token nextToken;
    Token forwardToken;
    string operatorTargetType;
    string errorString;
    size_t mode,
      nextInsertSubscript,
      lambdaObjectCount;
    size_t subscript;
    bool marked;

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
    bool SelfOperator(Message &msg);
    bool Colon();
    Object *GetObj(string name);
    static vector<string> Spilt(string target);
    static string GetHead(string target);
    static int GetPriority(string target);
    bool Assemble(Message &msg);
  public:
    Processor(): health(false), commaExpFunc(false), insertBtnSymbols(false), disableSetEntry(false),
                 dotOperator(false), defineLine(false), functionLine(false), subscriptProcessing(false), mode(0),
                 nextInsertSubscript(0), lambdaObjectCount(0) {}

    Processor &Reset() {
      Kit().CleanupVector(origin);
      return *this;
    }

    Message Start(size_t mode = kModeNormal);
    Processor &Build(string target);
    bool IsHealth() const { return health; }
    bool IsMarked() const { return marked; }
    string GetErrorString() const { return errorString; }
    bool IsSelfObjectManagement() const {
      string front = origin.front().first;
      return (front == kStrFor || front == kStrDef);
    }
    Processor &SetSubscript(size_t sub) {
      this->subscript = sub;
      marked = true; 
      return *this;
    }
    
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
    size_t ResetPlugin();
#else
    //Linux Version
#endif
    using EntryMapUnit = map<string, EntryProvider>::value_type;

    list<ObjectManager> &GetObjectStack();
    ObjectManager &GetCurrentManager();
    string GetTypeId(string sign);
    void Inject(EntryProvider provider);
    void RemoveByTemplate(ActivityTemplate temp);
    Object *FindObject(string name);
    ObjectManager &CreateManager();
    bool DisposeManager();
    EntryProvider Order(string id, string type, int size);
    std::wstring s2ws(const std::string& s);
  }
}


