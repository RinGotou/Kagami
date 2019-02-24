#pragma once
#include "ir.h"

namespace kagami {
  enum InterfaceType {
    kInterfaceTypePlain, kInterfaceTypeMethod
  };
  
  /* 
    Because of sturcture of classes, I place this function pointer here,
    and you can find that function(FunctionAgentTunnel()) in "module.cc".
  */
  using AgentActivity = Message(*)(ObjectMap &, vector<IR>);

  
  class InterfacePolicy {
  public:
    virtual Message Start(ObjectMap &p) = 0;
  };

  /* C++ function wrapper */
  class CXXFunctionPolicy : public InterfacePolicy {
  private:
    Activity activity_;
  public:
    CXXFunctionPolicy(Activity activity) :
      activity_(activity) {}

    Message Start(ObjectMap &p) { return activity_(p); }
  };

  /* Delegate IR from user-defined function */
  class IRFunctionPolicy : public InterfacePolicy {
  private:
    vector<IR> storage_;
    KIR ir_;
    AgentActivity agent_activity_;
  public:
    IRFunctionPolicy(vector<IR> storage, AgentActivity agent) :
      storage_(storage), agent_activity_(agent) {}

    Message Start(ObjectMap &p) { 
      return agent_activity_(p, storage_);
    }

    KIR &GetIR() {
      return ir_;
    }
  };

  enum InterfacePolicyType {
    kInterfaceCXX, kInterfaceIR
  };

  class Interface {
  private:
    shared_ptr<InterfacePolicy> policy_;
    ObjectMap closure_record_;

  private:
    string id_;
    GenericToken token_;
    vector<string> params_;
    StateCode argument_mode_;
    string domain_;
    InterfaceType interface_type_;
    InterfacePolicyType policy_type_;
    size_t min_arg_size_;

  public:
    Interface() :
      policy_(nullptr),
      id_(),
      token_(kTokenNull),
      params_(),
      argument_mode_(kCodeIllegalParam),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX),
      min_arg_size_(0) {}

    //Plain Function (CXX Type)
    Interface(
      Activity activity,
      string params,
      string id,
      StateCode argument_mode = kCodeNormalParam
    ) :
      policy_(make_shared<CXXFunctionPolicy>(activity)),
      id_(id),
      token_(kTokenNull),
      params_(BuildStringVector(params)),
      argument_mode_(argument_mode),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX),
      min_arg_size_(0) {}

    //Generic Token Function
    Interface(
      Activity activity,
      string params,
      GenericToken token,
      StateCode argument_mode = kCodeNormalParam
    ) :
      policy_(make_shared<CXXFunctionPolicy>(activity)),
      id_(),
      token_(token),
      params_(BuildStringVector(params)),
      argument_mode_(argument_mode),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX),
      min_arg_size_(0) {}

    //Plain Function (IR Type)
    Interface(
      vector<IR> ir_set,
      string id,
      vector<string> params,
      AgentActivity agent
    ) :
      policy_(make_shared<IRFunctionPolicy>(ir_set, agent)),
      id_(id),
      token_(kTokenNull),
      params_(params),
      argument_mode_(kCodeNormalParam),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceIR),
      min_arg_size_(0) {}

    Message Start(ObjectMap &obj_map) {
      Message result;
      ObjectMap combined_scope;
      
      if (!closure_record_.empty()) {
        combined_scope.merge(closure_record_);
      }

      combined_scope.merge(obj_map);

      switch (policy_type_) {
      case kInterfaceCXX:
        result = policy_->Start(combined_scope);
        break;
      case kInterfaceIR:
        combined_scope[kStrUserFunc] = Object(id_);
        result = policy_->Start(combined_scope);
        break;
      }

      return result;
    }

    bool operator==(Interface &rhs) const {
      return (
        policy_ == rhs.policy_ &&
        id_ == rhs.id_ &&
        token_ == rhs.token_ &&
        params_ == rhs.params_&&
        argument_mode_ == rhs.argument_mode_&&
        domain_ == rhs.domain_&&
        interface_type_ == rhs.interface_type_&&
        policy_type_ == rhs.policy_type_
        );
    }

    bool operator==(Interface &&rhs) const {
      return this->operator==(rhs);
    }

    bool operator!=(Interface &rhs) const {
      return !this->operator==(rhs);
    }

    bool operator!=(Interface &&rhs) const {
      return !this->operator==(rhs);
    }

    string GetTypeDomain() const {
      return domain_;
    }

    GenericToken GetToken() const {
      return token_;
    }

    string GetId() const {
      return id_;
    }

    StateCode GetArgumentMode() const {
      return argument_mode_;
    }

    vector<string> GetParameters() const {
      return params_;
    }

    InterfaceType GetInterfaceType() const {
      return interface_type_;
    }

    InterfacePolicyType GetPolicyType() const {
      return policy_type_;
    }

    KIR &GetIR() {
      return dynamic_pointer_cast<IRFunctionPolicy>(policy_)->GetIR();
    }

    size_t GetParamSize() const {
      return params_.size();
    }

    Interface &SetDomain(string domain) {
      domain_ = domain;
      return *this;
    }

    bool Good() const {
      return (policy_ != nullptr && argument_mode_ != kCodeIllegalParam);
    }

    Interface &SetClousureRecord(ObjectMap record) {
      closure_record_ = record;
      return *this;
    }

    Interface &SetMinArgSize(size_t size) {
      min_arg_size_ = size;
      return *this;
    }

    size_t GetMinArgSize() const {
      return min_arg_size_;
    }

    Interface &SetArgumentMode(StateCode code) {
      argument_mode_ = code;
      return *this;
    }
  };
}
