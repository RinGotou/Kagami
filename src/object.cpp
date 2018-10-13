#include "object.h"

namespace kagami {
  Object::Object() {
    //hold a null pointer will cause some mysterious ploblems,
    //so this will hold a specific value intead of nullptr
    ptr_ = nullptr;
    type_id_ = kTypeIdNull;
    token_type_ = TokenTypeEnum::T_NUL;
    ro_ = false;
    ref_ = false;
    constructor_ = false;
  }

  Object &Object::Manage(string t, TokenTypeEnum tokenType) {
    if (ref_) return GetTargetObject()->Manage(t, tokenType);
    ptr_ = std::make_shared<string>(t);
    type_id_ = kTypeIdRawString;
    methods_ = kRawStringMethods;
    token_type_ = tokenType;
    return *this;
  }

  Object &Object::Set(shared_ptr<void> ptr, string type_id) {
    if (ref_) return GetTargetObject()->Set(ptr, type_id);
    ptr_ = ptr;
    type_id_ = type_id;
    return *this;
  }

  Object &Object::Set(shared_ptr<void> ptr, string type_id, string methods, bool ro) {
    if (ref_) return GetTargetObject()->Set(ptr, type_id, methods, ro);
    ptr_ = ptr;
    type_id_ = type_id;
    methods_ = methods;
    ro_ = ro;
    return *this;
  }

  Object &Object::Ref(Object &object) {
    type_id_ = kTypeIdRef;
    ref_ = true;

    TargetObject target;
    if (!object.IsRef()) {
      target.ptr = &object;
    }
    else {
      target.ptr = 
        static_pointer_cast<TargetObject>(object.ptr_)->ptr;
    }
    ptr_ = make_shared<TargetObject>(target);
    return *this;
  }

  void Object::Clear() {
    ptr_ = make_shared<int>(0);
    type_id_ = kTypeIdNull;
    methods_.clear();
    token_type_ = TokenTypeEnum::T_NUL;
    ro_ = false;
    ref_ = false;
  }

  bool Object::Compare(Object &object) const {
    return (ptr_ == object.ptr_ &&
      type_id_ == object.type_id_ &&
      methods_ == object.methods_ &&
      token_type_ == object.token_type_ &&
      ro_ == object.ro_ &&
      constructor_ == object.constructor_ &&
      ref_ == object.ref_);
  }

  Object &Object::Copy(Object &object, bool force) {
    auto mod = [&]() {
      ptr_ = object.ptr_;
      type_id_ = object.type_id_;
      methods_ = object.methods_;
      token_type_ = object.token_type_;
      ro_ = object.ro_;
      ref_ = object.ref_;
      constructor_ = object.constructor_;
    };

    if (force) {
      mod();
    }
    else {
      if (ref_) GetTargetObject()->Copy(object);
      else mod();
    }
    return *this;
  }

  shared_ptr<void> Object::Get() {
    if (ref_) return GetTargetObject()->Get();
    return ptr_;
  }

  string Object::GetTypeId() {
    if (ref_) return GetTargetObject()->GetTypeId();
    return type_id_;
  }

  Object &Object::SetMethods(string methods) {
    if (ref_) return GetTargetObject()->SetMethods(methods);
    methods_ = methods;
    return *this;
  }

  Object &Object::AppendMethod(string method) {
    if (ref_) return GetTargetObject()->AppendMethod(method);
    methods_.append("|" + method);
    return *this;
  }

  Object &Object::SetTokenType(TokenTypeEnum token_type) {
    if (ref_) return GetTargetObject()->SetTokenType(token_type);
    token_type_ = token_type;
    return *this;
  }

  Object &Object::SetRo(bool ro) {
    if (ref_) return GetTargetObject()->SetRo(ro);
    ro_ = ro;
    return *this;
  }

  string Object::GetMethods() {
    if (ref_) return GetTargetObject()->GetMethods();
    return methods_;
  }

  TokenTypeEnum Object::GetTokenType() {
    if (ref_) return GetTargetObject()->GetTokenType();
    return token_type_;
  }

  bool Object::IsRo() {
    if (ref_) return GetTargetObject()->IsRo();
    return ro_;
  }

  bool Object::ConstructorFlag() {
    bool result = constructor_;
    constructor_ = false;
    return result;
  }

  bool ObjectContainer::Add(string sign, Object &source) {
    if (!CheckObject(sign)) return false;
    base.push_back(NamedObject(sign, Object().Copy(source)));
    return true;
  }

  Object *ObjectContainer::Find(string sign) {
    Object *object = nullptr;
    if (base.empty()) return object;
    for (size_t i = 0; i < base.size(); ++i) {
      if (base[i].first == sign) {
        object = &(base[i].second);
        break;
      }
    }
    return object;
  }

  void ObjectContainer::Dispose(string sign) {
    size_t pos = 0;
    bool found = false;
    for (size_t i = 0; i < base.size(); ++i) {
      if (base[i].first == sign) {
        found = true;
        pos = i;
        break;
      }
    }
    if (found) base.erase(pos);
  }

  void ObjectContainer::clear() {
    base.clear();
  }
}