#pragma once
#include "ir.h"

namespace kagami {
  enum InterfaceType {
    kInterfaceTypePlain, kInterfaceTypeMethod
  };

  using AgentActivity = Message(*)(ObjectMap &, vector<IR>);

  //TODO:Pending modify
  class InterfacePolicy {
  public:
    virtual Message Start(ObjectMap &p) = 0;
  };

  class CXXFunctionPolicy : public InterfacePolicy {
  private:
    Activity activity_;
  public:
    CXXFunctionPolicy(Activity activity) :
      activity_(activity) {}

    Message Start(ObjectMap &p) { return activity_(p); }
  };

  class IRFunctionPolicy : public InterfacePolicy {
  private:
    vector<IR> storage_;
    AgentActivity agent_activity_;
  public:
    IRFunctionPolicy(vector<IR> storage, AgentActivity agent) :
      storage_(storage), agent_activity_(agent) {}

    Message Start(ObjectMap &p) { 
      return agent_activity_(p, storage_);
    }
  };

  enum InterfacePolicyType {
    kInterfaceCXX, kInterfaceIR
  };

  class Interface {
  private:
    shared_ptr<InterfacePolicy> policy_;

  private:
    string id_;
    GenericToken token_;
    vector<string> params_;
    StateCode argument_mode_;
    string domain_;
    InterfaceType interface_type_;
    InterfacePolicyType policy_type_;

  public:
    Interface() :
      policy_(nullptr),
      id_(),
      token_(kTokenNull),
      params_(),
      argument_mode_(kCodeIllegalParam),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX) {}

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
      params_(util::BuildStringVector(params)),
      argument_mode_(argument_mode),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX) {}

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
      params_(util::BuildStringVector(params)),
      argument_mode_(argument_mode),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX) {}

    //Method (CXX Type)
    Interface(
      Activity activity,
      string params,
      string id,
      string domain,
      StateCode argument_mode = kCodeNormalParam
    ) :
      policy_(make_shared<CXXFunctionPolicy>(activity)),
      id_(id),
      token_(kTokenNull),
      params_(util::BuildStringVector(params)),
      argument_mode_(argument_mode),
      domain_(domain),
      interface_type_(kInterfaceTypeMethod),
      policy_type_(kInterfaceCXX) {}

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
      policy_type_(kInterfaceIR) {}

    Message Start(ObjectMap &obj_map) {
      Message result;

      switch (policy_type_) {
      case kInterfaceCXX:
        result = policy_->Start(obj_map);
        break;
      case kInterfaceIR:
        obj_map[kStrUserFunc] = Object(id_);
        result = policy_->Start(obj_map);
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

    GenericToken GetTokenEnum() const {
      return token_;
    }

    string GetId() const {
      return id_;
    }

    int GetArgumentMode() const {
      return argument_mode_;
    }

    vector<string> GetParameters() const {
      return params_;
    }

    InterfaceType GetInterfaceType() const {
      return interface_type_;
    }

    size_t GetParamSize() const {
      return params_.size();
    }

    bool Good() const {
      return (policy_ != nullptr && argument_mode_ != kCodeIllegalParam);
    }
  };
}