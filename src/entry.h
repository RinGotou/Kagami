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

    Message Start(ObjectMap &obj_map) const {
      Message result;
      if (is_user_func_) {
        obj_map[kStrUserFunc] = Object(id_, T_GENERIC);
      }
      if (Good()) {
        result = activity_(obj_map);
      }
      else {
        result = Message(kStrFatalError, kCodeIllegalCall, "Illegal entry.");
      }
      return result;
    }

    bool operator==(Entry &target) const { 
      return (target.id_ == id_ &&
        target.activity_ == activity_ &&
        target.argument_mode_ == argument_mode_ &&
        target.type_ == type_ &&
        target.parms_ == parms_);
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
}