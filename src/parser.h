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

  class EntryProvider;
  class Chainloader;

  /*Core Class
  this class contains all main functions of script processor.
  */
  class Core {
  private:
    bool ignore_fatal_error;
  public:
    //TODO:startup arugment
    void Terminal();
    void PrintEvents();
    Message ExecScriptFile(string target);
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
    Object() { 
      ptr = nullptr; 
      option = kTypeIdNull;
      tag = kStrEmpty;
    }
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

    Object GetObj(string name);
    vector<string> spilt(string target);
    string GetHead(string target);
    int GetPriority(string target) const;
    bool Assemble(bool disable_set_entry, deque<string> &item, deque<string> &symbol,
      Message &msg, size_t mode);
  public:
    Chainloader() {}
    Chainloader &Build(vector<string> raw) {
      this->raw = raw;
      return *this;
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
    string id;
    int arg_mode;
    int priority;
    vector<string> args;
    Activity activity;
    string specifictype;
  public:
    EntryProvider() : id(kStrNull), activity(nullptr) {
      arg_mode = kCodeIllegalArgs;
      specifictype = kTypeIdNull;
    }

    EntryProvider(ActivityTemplate temp) :
    id(temp.id), args(Kit().BuildStringVector(temp.args)), 
      activity(temp.activity) {
      arg_mode = temp.arg_mode;
      priority = temp.priority;
      specifictype = temp.specifictype;
    }

    bool operator==(EntryProvider &target) {
      return (target.id == this->id &&
        target.activity == this->activity &&
        target.arg_mode == this->arg_mode &&
        target.priority == this->priority &&
        this->specifictype == target.specifictype &&
        target.args == this->args);
    }

    bool operator==(ActivityTemplate &target) {
      return(
        this->id == target.id &&
        this->arg_mode == target.arg_mode &&
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
    int GetArgumentMode() const { return this->arg_mode; }
    size_t GetArgumentSize() const { return this->args.size(); }
    int GetPriority() const { return this->priority; }
    bool Good() const { return ((activity != nullptr) && arg_mode != kCodeIllegalArgs); }
    Message StartActivity(deque<Object> p, Chainloader *parent);
  };

  inline string CastToString(shared_ptr<void> ptr) {
    return *static_pointer_cast<string>(ptr);
  }

  void Activiate();
  void InitTemplates();

  namespace type {
    ObjTemplate *GetTemplate(string name);
    void AddTemplate(string name, ObjTemplate temp);
  }

  namespace trace {
    using log_t = pair<string, Message>;
    void log(kagami::Message msg);
  }

  namespace entry {
#if defined(_WIN32)
    //Windows Verison
    class Instance : public pair<string, HINSTANCE> {
    private:
      bool health;
      vector<ActivityTemplate> act_temp;
    public:
      Instance() { health = false; }
      bool Load(string name, HINSTANCE h);
      bool GetHealth() const { return health; }
      vector<ActivityTemplate> GetMap() const { return act_temp; }
      CastAttachment getObjTemplate() { return (CastAttachment)GetProcAddress(this->second, "CastAttachment"); }
      MemoryDeleter getDeleter() { return (MemoryDeleter)GetProcAddress(this->second, "FreeMemory"); }
    };
#else
    //Linux Version
#endif
    using EntryMapUnit = map<string, EntryProvider>::value_type;
    
    string GetTypeId(string sign);
    std::wstring s2ws(const std::string& s);
    void Inject(EntryProvider provider);
    void RemoveByTemplate(ActivityTemplate temp);
    Object *FindObject(string name);
    ObjectManager &CreateManager();
    bool DisposeManager();
    size_t ResetPlugin();
  }
}


