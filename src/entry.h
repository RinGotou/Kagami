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
    int parmMode, priority, flag;
    vector<string> args;
    Activity activity;
    string type;
    bool placeholder, userFunc;
  public:
    Entry() : id(kStrNull), priority(0), activity(nullptr), flag(kFlagNormalEntry) {
      parmMode = kCodeIllegalParm;
      type = kTypeIdNull;
      placeholder = false;
      userFunc = false;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, int parmMode, string args,string id = kStrEmpty, string type = kTypeIdNull, int flag = kFlagNormalEntry, int priority = 4) :
      id(id),parmMode(parmMode), priority(priority), args(Kit::BuildStringVector(args)),
      type(type), flag(flag) {
      this->activity = activity;
      placeholder = false;
      userFunc = false;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, string args, GenericTokenEnum tokenEnum, int parmMode = kCodeNormalParm, int priority = 4) :
      id(), parmMode(parmMode), priority(priority), args(Kit::BuildStringVector(args)) {
      this->activity = activity;
      this->tokenEnum = tokenEnum;
      userFunc = false;
      placeholder = false;
    }

    Entry(string id) :id(id), priority(0), activity(nullptr) {
      parmMode = kCodeNormalParm;
      type = kTypeIdNull;
      userFunc = false;
      placeholder = true;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, string id,vector<string> args) : priority(4) {
      type = kTypeIdNull;
      placeholder = false;
      parmMode = kCodeNormalParm;
      flag = kFlagNormalEntry;
      userFunc = true;
      this->activity = activity;
      this->id = id;
      this->args = args;
      type = kTypeIdNull;
    }

    bool Compare(Entry &target) const;
    Message Start(ObjectMap &map) const;

    bool operator==(Entry &target) const { return Compare(target); }
    string GetSpecificType() const { return type; }
    GenericTokenEnum GetTokenEnum() const { return tokenEnum; }
    string GetId() const { return this->id; }
    int GetArgumentMode() const { return this->parmMode; }
    vector<string> GetArguments() const { return args; }
    size_t GetParmSize() const { return this->args.size(); }
    int GetPriority() const { return this->priority; }
    int GetFlag() const { return flag; }
    bool Good() const { 
      return ((activity != nullptr)
        && parmMode != kCodeIllegalParm 
        || (userFunc && id!=kStrEmpty));
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

    list<ObjectManager> &GetObjectStack();
    ObjectManager &GetCurrentManager();
    string GetTypeId(string sign);
    void AddEntry(Entry temp);
    void AddGenericEntry(GenericTokenEnum token, Entry temp);
    Object *FindObject(string name);
    ObjectManager &CreateManager();
    bool DisposeManager();
    Entry Order(string id, string type, int size);
    Object *FindObjectInCurrentManager(string sign);
    Object *CreateObject(string sign, Object &object);
    GenericTokenEnum GetGenericToken(string src);
    string GetGenTokenValue(GenericTokenEnum token);
    OperatorCode GetOperatorCode(string src);
    Entry Order(string id, string type = kTypeIdNull, int size = -1);
    bool IsOperatorToken(GenericTokenEnum token);
  }

  namespace type {
    ObjectPlanner *GetPlanner(string name);
    void AddTemplate(string name, ObjectPlanner temp);
    shared_ptr<void> GetObjectCopy(Object &object);
  }
}