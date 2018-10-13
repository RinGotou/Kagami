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
    vector<string> parms;
    int argumentMode, priority;
    string type;
    int flag;
    bool isPlaceholder, isUserFunc, needRecheck, isMethod;
  public:
    Entry() : id(kStrNull), 
      activity(nullptr), 
      priority(0), 
      flag(kFlagNormalEntry) {

      argumentMode = kCodeIllegalParm;
      type = kTypeIdNull;
      isPlaceholder = false;
      isUserFunc = false;
      needRecheck = false;
      isMethod = false;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      int argumentMode, 
      string parms,
      string id = kStrEmpty, 
      string type = kTypeIdNull, 
      int flag = kFlagNormalEntry, 
      int priority = 4) :
      id(id), 
      parms(Kit::BuildStringVector(parms)), 
      argumentMode(argumentMode), 
      priority(priority),
      type(type), 
      flag(flag) {

      this->activity = activity;
      isPlaceholder = false;
      isUserFunc = false;
      needRecheck = false;
      isMethod = false;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      string parms, 
      GenericTokenEnum tokenEnum, 
      int argumentMode = kCodeNormalParm, 
      int priority = 4) :
      id(), 
      parms(Kit::BuildStringVector(parms)), 
      argumentMode(argumentMode), 
      priority(priority) {

      this->activity = activity;
      this->tokenEnum = tokenEnum;
      isUserFunc = false;
      needRecheck = false;
      isPlaceholder = false;
      isMethod = false;
    }

    Entry(string id) :
      id(id), 
      activity(nullptr), 
      priority(0) {

      argumentMode = kCodeNormalParm;
      type = kTypeIdNull;
      isUserFunc = false;
      needRecheck = false;
      isPlaceholder = true;
      isMethod = false;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      string id,
      vector<string> parms) : 
      priority(4) {

      type = kTypeIdNull;
      isPlaceholder = false;
      argumentMode = kCodeNormalParm;
      flag = kFlagNormalEntry;
      isUserFunc = true;
      this->activity = activity;
      this->id = id;
      needRecheck = false;
      this->parms = parms;
      isMethod = false;
      type = kTypeIdNull;
    }

    void SetRecheckInfo(string id,bool isMethod = false) {
      this->id = id;
      this->isMethod = isMethod;
      needRecheck = true;
      tokenEnum = GenericTokenEnum::GT_NUL;
    }

    bool Compare(Entry &target) const;
    Message Start(ObjectMap &map) const;

    bool operator==(Entry &target) const { 
      return Compare(target); 
    }

    Entry &SetSpecificTypeSign() { 
      this->isMethod = true; 
      return *this; 
    }

    bool IsMethod() const { 
      return isMethod; 
    }

    string GetSpecificType() const { 
      return type; 
    }

    GenericTokenEnum GetTokenEnum() const { 
      return tokenEnum; 
    }

    bool NeedRecheck() const { 
      return needRecheck; 
    }

    string GetId() const { 
      return this->id; 
    }

    int GetArgumentMode() const { 
      return this->argumentMode; 
    }

    vector<string> GetArguments() const { 
      return parms; 
    }

    size_t GetParmSize() const { 
      return this->parms.size(); 
    }

    int GetPriority() const { 
      return this->priority; 
    }

    int GetFlag() const {
      return flag; 
    }

    bool Good() const { 
      bool conditionA =
        ((activity != nullptr) && (argumentMode != kCodeIllegalParm));
      bool conditionB =
        (isUserFunc && id != kStrEmpty);

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