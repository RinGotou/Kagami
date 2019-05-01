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

  /* VMCode Delegator for Kisaragi framework */
  class VMCodePolicy : public InterfacePolicy {
  private:
    VMCode code_;

  public:
    VMCodePolicy(VMCode ir) : code_(ir) {}

    Message Start(ObjectMap &p) { return Message(); } //Deprecated method

    VMCode &GetCode() { return code_; }
  };

  enum InterfacePolicyType {
    kInterfaceCXX, kInterfaceVMCode
  };

  class Interface {
  private:
    shared_ptr<InterfacePolicy> policy_;
    ObjectMap closure_record_;

  private:
    string id_;
    Keyword token_;
    vector<string> params_;
    StateCode argument_mode_;
    string domain_;
    InterfaceType interface_type_;
    InterfacePolicyType policy_type_;
    size_t min_arg_size_;
    size_t offset_;

  public:
    Interface() :
      policy_(nullptr),
      id_(),
      token_(kKeywordNull),
      params_(),
      argument_mode_(kCodeIllegalParam),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX),
      min_arg_size_(0),
      offset_(0) {}

    Interface(
      Activity activity,
      string params,
      string id,
      StateCode argument_mode = kCodeNormalParam
    ) :
      policy_(new CXXFunctionPolicy(activity)),
      id_(id),
      token_(kKeywordNull),
      params_(BuildStringVector(params)),
      argument_mode_(argument_mode),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceCXX),
      min_arg_size_(0),
      offset_(0) {}

    Interface(
      size_t offset,
      VMCode ir,
      string id,
      vector<string> params,
      StateCode argument_mode = kCodeNormalParam
    ) :
      policy_(new VMCodePolicy(ir)),
      id_(id),
      token_(kKeywordNull),
      params_(params),
      argument_mode_(argument_mode),
      domain_(kTypeIdNull),
      interface_type_(kInterfaceTypePlain),
      policy_type_(kInterfaceVMCode),
      min_arg_size_(0),
      offset_(offset) {}

    Message Start(ObjectMap &obj_map) {
      Message result;

      result = static_pointer_cast<CXXFunctionPolicy>(policy_)
        ->Start(obj_map);

      return result;
    }

    bool operator==(Interface &rhs) const {
      if (&rhs == this) return true;
      return policy_ == rhs.policy_;
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

    Keyword GetToken() const {
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

    VMCode &GetCode() {
      return dynamic_pointer_cast<VMCodePolicy>(policy_)->GetCode();
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

    ObjectMap &GetClosureRecord() {
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

    size_t GetOffset() const {
      return offset_;
    }
  };

  Message MakeInvokePoint(string id, string type_id = kTypeIdNull);

  using InterfacePointer = Interface *;
}
