#pragma once
#include "util.h"
#include "message.h"

namespace kagami {
  enum InterfaceType {
    kEntryNormal, kEntryMethod
  };

  class Interface {
    string id_;
    GenericTokenEnum gen_token_;
    Activity activity_;
    vector<string> params_;
    StateCode argument_mode_;
    string type_;
    InterfaceType interface_type_;
    bool is_user_func_;
  public:
    /* Empty interface */
    Interface() : id_(),
      gen_token_(GT_NUL),
      activity_(nullptr),
      params_(),
      argument_mode_(kCodeIllegalParam),
      type_(kTypeIdNull),
      interface_type_(kEntryNormal),
      is_user_func_(false) {}

    /* Plain function */
    Interface(Activity activity,
      string params,
      string id,
      StateCode argument_mode = kCodeNormalParam) :
      id_(id),
      gen_token_(GT_NUL),
      activity_(activity),
      params_(util::BuildStringVector(params)),
      argument_mode_(argument_mode),
      type_(kTypeIdNull),
      interface_type_(kEntryNormal),
      is_user_func_(false) {}

    /* Method */
    Interface(Activity activity,
      string params,
      string id,
      string type,
      StateCode argument_mode = kCodeNormalParam) :
      id_(id),
      gen_token_(GT_NUL),
      activity_(activity),
      params_(util::BuildStringVector(params)),
      argument_mode_(argument_mode),
      type_(type),
      interface_type_(kEntryMethod),
      is_user_func_(false) {}

    /* Generic token function */
    Interface(Activity activity,
      string params,
      GenericTokenEnum token,
      StateCode argumentMode = kCodeNormalParam) :
      id_(),
      gen_token_(token),
      activity_(activity),
      params_(util::BuildStringVector(params)),
      argument_mode_(argumentMode),
      type_(kTypeIdNull),
      interface_type_(kEntryMethod),
      is_user_func_(false) {}

    /* user-defined function */
    Interface(Activity activity,
      string id,
      vector<string> params) :
      id_(id),
      gen_token_(GT_NUL),
      activity_(activity),
      params_(params),
      argument_mode_(kCodeNormalParam),
      type_(kTypeIdNull),
      interface_type_(kEntryNormal),
      is_user_func_(true) {}

    Message Start(ObjectMap &obj_map) const {
      Message result;
      if (is_user_func_) {
        obj_map[kStrUserFunc] = Object(id_);
      }
      if (Good()) {
        result = activity_(obj_map);
      }
      else {
        result = Message(kCodeIllegalCall, "Invalid interface.", kStateError);
      }
      return result;
    }

    bool operator==(Interface &target) const { 
      return (target.id_ == id_ &&
        target.activity_ == activity_ &&
        target.argument_mode_ == argument_mode_ &&
        target.type_ == type_ &&
        target.params_ == params_);
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
      return params_; 
    }

    size_t GetParamSize() const { 
      return params_.size(); 
    }

    InterfaceType GetEntryType() const {
      return interface_type_;
    }

    bool Good() const { 
      bool conditionA =
        ((activity_ != nullptr) && (argument_mode_ != kCodeIllegalParam));
      bool conditionB =
        (is_user_func_ && id_ != "");

      return (conditionA || conditionB);
    }
  };
}