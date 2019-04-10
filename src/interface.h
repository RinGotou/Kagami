#pragma once
#include "ir.h"

namespace kagami {
  enum InterfaceType {
    kInterfaceTypePlain, kInterfaceTypeMethod
  };

  
  class InterfacePolicy {
  public:
    virtual ~InterfacePolicy() {}
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

  /* KIR Delegator for Kisaragi framework */
  class KIRFunctionPolicy : public InterfacePolicy {
  private:
    KIR ir_;

  public:
    KIRFunctionPolicy(KIR ir) : ir_(ir) {}

    Message Start(ObjectMap &p) { return Message(); } //Deprecated method

    KIR &GetIR() { return ir_; }
  };

  enum InterfacePolicyType {
    kInterfaceCXX, kInterfaceKIR
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
      policy_(new CXXFunctionPolicy(activity)),
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
      policy_(new CXXFunctionPolicy(activity)),
      id_(),
      token_(token),
      params_(BuildStringVector(params)),
      argument_mode_(argument_mode),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX),
      min_arg_size_(0) {}

    Interface(
      KIR ir,
      string id,
      vector<string> params,
      StateCode argument_mode = kCodeNormalParam
    ) :
      policy_(new KIRFunctionPolicy(ir)),
      id_(id),
      token_(kTokenNull),
      params_(params),
      argument_mode_(argument_mode),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceKIR),
      min_arg_size_(0) {}

    Message Start(ObjectMap &obj_map) {
      Message result;

      result = static_pointer_cast<CXXFunctionPolicy>(policy_)
        ->Start(obj_map);

      return result;
    }

    bool operator==(Interface &rhs) const {
      if (&rhs == this) return true;
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
      if (&rhs == this) return false;
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

    vector<string> &GetParameters() {
      return params_;
    }

    InterfaceType GetInterfaceType() const {
      return interface_type_;
    }

    InterfacePolicyType GetPolicyType() const {
      return policy_type_;
    }

    KIR &GetIR() {
      return dynamic_pointer_cast<KIRFunctionPolicy>(policy_)->GetIR();
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

    Interface &SetClosureRecord(ObjectMap record) {
      closure_record_ = record;
      return *this;
    }

    ObjectMap GetClosureRecord() {
      return closure_record_;
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

  Message MakeInvokePoint(string id, string type_id = kTypeIdNull);

  using InterfacePointer = Interface *;
}
