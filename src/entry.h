#pragma once
#include "util.h"
#include "message.h"

namespace kagami {
  class Entry {
    string id_;
    GenericTokenEnum gen_token_;
    Activity activity_;
    vector<string> parms_;
    int argument_mode_;
    string type_;
    int flag_;
    bool is_user_func_;
  public:
    /* Empty entry */
    Entry() : id_(kStrNull),
      gen_token_(GT_NUL),
      activity_(nullptr),
      parms_(),
      argument_mode_(kCodeIllegalParm),
      type_(kTypeIdNull),
      flag_(kFlagNormalEntry),
      is_user_func_(false) {}

    /* Plain function */
    Entry(Activity activity,
      string parms,
      string id,
      int argument_mode = kCodeNormalParm) :
      id_(id),
      gen_token_(GT_NUL),
      activity_(activity),
      parms_(util::BuildStringVector(parms)),
      argument_mode_(argument_mode),
      type_(kTypeIdNull),
      flag_(kFlagNormalEntry),
      is_user_func_(false) {}

    /* Method */
    Entry(Activity activity,
      string parms,
      string id,
      string type,
      int argument_mode = kCodeNormalParm) :
      id_(id),
      gen_token_(GT_NUL),
      activity_(activity),
      parms_(util::BuildStringVector(parms)),
      argument_mode_(argument_mode),
      type_(type),
      flag_(kFlagMethod),
      is_user_func_(false) {}

    /* Generic token function */
    Entry(Activity activity,
      string parms,
      GenericTokenEnum token,
      int argumentMode = kCodeNormalParm) :
      id_(),
      gen_token_(token),
      activity_(activity),
      parms_(util::BuildStringVector(parms)),
      argument_mode_(argumentMode),
      type_(kTypeIdNull),
      flag_(kFlagNormalEntry),
      is_user_func_(false) {}

    /* user-defined function */
    Entry(Activity activity,
      string id,
      vector<string> parms) :
      id_(id),
      gen_token_(GT_NUL),
      activity_(activity),
      parms_(parms),
      argument_mode_(kCodeNormalParm),
      type_(kTypeIdNull),
      flag_(kFlagNormalEntry),
      is_user_func_(true) {}

    bool Compare(Entry &target) const;
    Message Start(ObjectMap &map) const;

    bool operator==(Entry &target) const { 
      return Compare(target); 
    }

    string GetTypeDomain() const { 
      return type_; 
    }

    GenericTokenEnum GetTokenEnum() const { 
      return gen_token_; 
    }

    string GetId() const { 
      return id_; 
    }

    int GetArgumentMode() const { 
      return argument_mode_; 
    }

    vector<string> GetArguments() const { 
      return parms_; 
    }

    size_t GetParmSize() const { 
      return parms_.size(); 
    }

    int GetFlag() const {
      return flag_; 
    }

    bool Good() const { 
      bool conditionA =
        ((activity_ != nullptr) && (argument_mode_ != kCodeIllegalParm));
      bool conditionB =
        (is_user_func_ && id_ != kStrEmpty);

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
    Object *FindObjectInCurrentContainer(string id);
    Object *FindObject(string id);
    Object *CreateObject(string id, Object &object);

    string GetTypeId(string id);
    string GetGenTokenValue(GenericTokenEnum token);
    void AddEntry(Entry temp);
    void AddGenericEntry(Entry temp);
    
    bool DisposeManager();
    bool IsOperatorToken(GenericTokenEnum token);
    bool HasTailTokenRequest(GenericTokenEnum token);

    Entry Order(string id, string type = kTypeIdNull, int size = -1);
    GenericTokenEnum GetGenericToken(string src);
    OperatorCode GetOperatorCode(string src);
    Entry GetGenericProvider(GenericTokenEnum token);
    int GetTokenPriority(GenericTokenEnum token);
    bool IsMonoOperator(GenericTokenEnum token);
  }

  namespace type {
    ObjectCopyingPolicy *GetPlanner(string name);
    string GetMethods(string name);
    void AddTemplate(string name, ObjectCopyingPolicy temp);
    shared_ptr<void> GetObjectCopy(Object &object);
  }
}