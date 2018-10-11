#pragma once
#include "kit.h"
#include "message.h"

namespace kagami {
  /*Entry Class
  contains function pointer.Processed argument tokens are used for building
  new argument map.entry provider have two mode:internal function and plugin
  function.
  */

  class Entry {
    string id;
    GenericTokenEnum tokenEnum;
    Activity activity;
    vector<string> args;
    int parmMode, priority;
    string type;
    int flag;
    bool placeholder, userFunc, entrySign, method;
  public:
    Entry() : id(kStrNull), 
      activity(nullptr), 
      priority(0), 
      flag(kFlagNormalEntry) {

      parmMode = kCodeIllegalParm;
      type = kTypeIdNull;
      placeholder = false;
      userFunc = false;
      entrySign = false;
      method = false;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      int parmMode, 
      string args,
      string id = kStrEmpty, 
      string type = kTypeIdNull, 
      int flag = kFlagNormalEntry, 
      int priority = 4) :
      id(id), 
      args(Kit::BuildStringVector(args)), 
      parmMode(parmMode), 
      priority(priority),
      type(type), 
      flag(flag) {

      this->activity = activity;
      placeholder = false;
      userFunc = false;
      entrySign = false;
      method = false;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      string args, 
      GenericTokenEnum tokenEnum, 
      int parmMode = kCodeNormalParm, 
      int priority = 4) :
      id(), 
      args(Kit::BuildStringVector(args)), 
      parmMode(parmMode), 
      priority(priority) {

      this->activity = activity;
      this->tokenEnum = tokenEnum;
      userFunc = false;
      entrySign = false;
      placeholder = false;
      method = false;
    }

    Entry(string id) :
      id(id), 
      activity(nullptr), 
      priority(0) {

      parmMode = kCodeNormalParm;
      type = kTypeIdNull;
      userFunc = false;
      entrySign = false;
      placeholder = true;
      method = false;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      string id,
      vector<string> args) : 
      priority(4) {

      type = kTypeIdNull;
      placeholder = false;
      parmMode = kCodeNormalParm;
      flag = kFlagNormalEntry;
      userFunc = true;
      this->activity = activity;
      this->id = id;
      entrySign = false;
      this->args = args;
      method = false;
      type = kTypeIdNull;
    }

    void SetEntrySign(string id,bool method = false) {
      this->id = id;
      this->method = method;
      entrySign = true;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    bool Compare(Entry &target) const;
    Message Start(ObjectMap &map) const;

    bool operator==(Entry &target) const { return Compare(target); }
    Entry &SetSpecificTypeSign() { this->method = true; return *this; }
    bool NeedSpecificType() const { return method; }
    string GetSpecificType() const { return type; }
    GenericTokenEnum GetTokenEnum() const { return tokenEnum; }
    bool GetEntrySign() const { return entrySign; }
    string GetId() const { return this->id; }
    int GetArgumentMode() const { return this->parmMode; }
    vector<string> GetArguments() const { return args; }
    size_t GetParmSize() const { return this->args.size(); }
    int GetPriority() const { return this->priority; }
    int GetFlag() const { return flag; }
    bool Good() const { 
      bool conditionA = 
        ((activity != nullptr) && (parmMode != kCodeIllegalParm)),
        conditionB = 
        (userFunc && id != kStrEmpty);
      return (conditionA || conditionB);
    }
  };

  namespace entry {
    enum OperatorCode {
      ADD, SUB, MUL, DIV, EQUAL, IS,
      MORE, LESS, NOT_EQUAL, MORE_OR_EQUAL, LESS_OR_EQUAL,
      SELFINC, SELFDEC, AND, OR, NOT, BIT_AND, BIT_OR, 
      NUL
    };

    OperatorCode GetOperatorCode(string src);

    using EntryMapUnit = map<string, Entry>::value_type;

    ContainerPool &GetContainerPool();
    ObjectContainer &GetCurrentContainer();
    ObjectContainer &CreateContainer();
    Object *FindObjectInCurrentContainer(string sign);
    Object *FindObject(string name);
    Object *CreateObject(string sign, Object &object);

    string GetTypeId(string sign);
    string GetGenTokenValue(GenericTokenEnum token);
    void AddEntry(Entry temp);
    void AddGenericEntry(Entry temp);
    
    bool DisposeManager();
    bool IsOperatorToken(GenericTokenEnum token);
    bool HasTailTokenRequest(GenericTokenEnum token);

    Entry Order(string id, string type = kTypeIdNull, int size = -1);
    GenericTokenEnum GetGenericToken(string src);
    OperatorCode GetOperatorCode(string src);
    

  }

  namespace type {
    ObjectPlanner *GetPlanner(string name);
    string GetMethods(string name);
    void AddTemplate(string name, ObjectPlanner temp);
    shared_ptr<void> GetObjectCopy(Object &object);
  }
}