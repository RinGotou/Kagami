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
    string id_;
    GenericTokenEnum gen_token_;
    Activity activity_;
    vector<string> parms_;
    int argument_mode_, priority_;
    string type_;
    int flag_;
    bool is_placeholder_, is_user_func_, need_recheck_, is_method_;
  public:
    Entry() : id_(kStrNull), 
      activity_(nullptr), 
      priority_(0), 
      flag_(kFlagNormalEntry) {

      argument_mode_ = kCodeIllegalParm;
      type_ = kTypeIdNull;
      is_placeholder_ = false;
      is_user_func_ = false;
      need_recheck_ = false;
      is_method_ = false;
      gen_token_ = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      int argumentMode, 
      string parms,
      string id = kStrEmpty, 
      string type = kTypeIdNull, 
      int flag = kFlagNormalEntry, 
      int priority = 4) :
      id_(id), 
      parms_(kit::BuildStringVector(parms)), 
      argument_mode_(argumentMode), 
      priority_(priority),
      type_(type), 
      flag_(flag) {

      activity_ = activity;
      is_placeholder_ = false;
      is_user_func_ = false;
      need_recheck_ = false;
      is_method_ = false;
      gen_token_ = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      string parms, 
      GenericTokenEnum tokenEnum, 
      int argumentMode = kCodeNormalParm, 
      int priority = 4) :
      id_(), 
      parms_(kit::BuildStringVector(parms)), 
      argument_mode_(argumentMode), 
      priority_(priority) {

      activity_ = activity;
      gen_token_ = tokenEnum;
      is_user_func_ = false;
      need_recheck_ = false;
      is_placeholder_ = false;
      is_method_ = false;
    }

    Entry(string id) :
      id_(id), 
      activity_(nullptr), 
      priority_(0) {

      argument_mode_ = kCodeNormalParm;
      type_ = kTypeIdNull;
      is_user_func_ = false;
      need_recheck_ = false;
      is_placeholder_ = true;
      is_method_ = false;
      gen_token_ = GenericTokenEnum::GT_NUL;
    }

    Entry(Activity activity, 
      string id,
      vector<string> parms) : 
      priority_(4) {

      type_ = kTypeIdNull;
      is_placeholder_ = false;
      argument_mode_ = kCodeNormalParm;
      flag_ = kFlagNormalEntry;
      is_user_func_ = true;
      activity_ = activity;
      id_ = id;
      need_recheck_ = false;
      parms_ = parms;
      is_method_ = false;
      type_ = kTypeIdNull;
    }

    void SetRecheckInfo(string id,bool isMethod = false) {
      id_ = id;
      is_method_ = isMethod;
      need_recheck_ = true;
      gen_token_ = GenericTokenEnum::GT_NUL;
    }

    bool Compare(Entry &target) const;
    Message Start(ObjectMap &map) const;

    bool operator==(Entry &target) const { 
      return Compare(target); 
    }

    Entry &set_is_method() { 
      is_method_ = true; 
      return *this; 
    }

    bool IsMethod() const { 
      return is_method_; 
    }

    string GetTypeDomain() const { 
      return type_; 
    }

    GenericTokenEnum GetTokenEnum() const { 
      return gen_token_; 
    }

    bool NeedRecheck() const { 
      return need_recheck_; 
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

    int GetPriority() const { 
      return priority_; 
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