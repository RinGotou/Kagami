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
    int parmMode;
    int priority;
    vector<string> args;
    Activity activity;
    string specifictype;
    size_t minsize;
  public:
    Entry() : id(kStrNull), priority(0), activity(nullptr), minsize(0) {
      parmMode = kCodeIllegalParm;
      specifictype = kTypeIdNull;
    }

    Entry(string id, Activity activity, int priority, int parmMode, string args, string type = kTypeIdNull) :
      id(id),parmMode(parmMode), priority(priority), args(Kit().BuildStringVector(args)),
      specifictype(type) {
      this->activity = activity;
    }

    bool Compare(Entry &target) const;
    Entry &SetSpecificType(string type);

    bool operator==(Entry &target) const { return Compare(target); }
    string GetSpecificType() const { return specifictype; }
    string GetId() const { return this->id; }
    int GetArgumentMode() const { return this->parmMode; }
    vector<string> GetArguments() const { return args; }
    size_t GetParameterSIze() const { return this->args.size(); }
    int GetPriority() const { return this->priority; }
    bool Good() const { return ((activity != nullptr) && parmMode != kCodeIllegalParm); }
    Message Start(ObjectMap &map) const;
  };

  namespace entry {
    enum OperatorCode {
      ADD, SUB, MUL, DIV, EQUAL, IS, NOT,
      MORE, LESS, NOT_EQUAL, MORE_OR_EQUAL, LESS_OR_EQUAL,
      SELFINC, SELFDEC,
      NUL
    };

    OperatorCode GetOperatorCode(string src);

    using EntryMapUnit = map<string, Entry>::value_type;

    list<ObjectManager> &GetObjectStack();
    ObjectManager &GetCurrentManager();
    string GetTypeId(string sign);
    void Inject(Entry temp);
    void LoadGenProvider(GenericTokenEnum token, Entry temp);
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
    size_t GetRequiredCount(string id);
  }
}