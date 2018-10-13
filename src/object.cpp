#include "object.h"

namespace kagami {
  Object::Object() {
    //hold a null pointer will cause some mysterious ploblems,
    //so this will hold a specific value intead of nullptr
    ptr = nullptr;
    parent = nullptr;
    type_id = kTypeIdNull;
    tokenTypeEnum = TokenTypeEnum::T_NUL;
    ro = false;
    ref = false;
    constructor = false;
  }

  Object &Object::Manage(string t, TokenTypeEnum tokenType) {
    if (ref) return GetTargetObject()->Manage(t, tokenType);
    this->ptr = std::make_shared<string>(t);
    this->type_id = kTypeIdRawString;
    this->methods = kRawStringMethods;
    this->tokenTypeEnum = tokenType;
    return *this;
  }

  Object &Object::Set(shared_ptr<void> ptr, string type_id) {
    if (ref) return GetTargetObject()->Set(ptr, type_id);
    this->ptr = ptr;
    this->type_id = type_id;
    return *this;
  }

  Object &Object::Set(shared_ptr<void> ptr, string type_id, string methods, bool ro) {
    if (ref) return GetTargetObject()->Set(ptr, type_id, methods, ro);
    this->ptr = ptr;
    this->type_id = type_id;
    this->methods = methods;
    this->ro = ro;
    return *this;
  }

  Object &Object::Ref(Object &object) {
    this->type_id = kTypeIdRef;
    this->ref = true;

    TargetObject target;
    if (!object.IsRef()) {
      target.ptr = &object;
    }
    else {
      target.ptr = static_pointer_cast<TargetObject>(object.ptr)->ptr;
    }
    ptr = make_shared<TargetObject>(target);
    return *this;
  }

  void Object::Clear() {
    ptr = make_shared<int>(0);
    type_id = kTypeIdNull;
    methods.clear();
    tokenTypeEnum = TokenTypeEnum::T_NUL;
    ro = false;
    ref = false;
  }

  bool Object::Compare(Object &object) const {
    return (ptr == object.ptr &&
      type_id == object.type_id &&
      methods == object.methods &&
      tokenTypeEnum == object.tokenTypeEnum &&
      ro == object.ro &&
      constructor == object.constructor &&
      ref == object.ref);
  }

  Object &Object::Copy(Object &object, bool force) {
    auto mod = [&]() {
      ptr = object.ptr;
      type_id = object.type_id;
      methods = object.methods;
      tokenTypeEnum = object.tokenTypeEnum;
      ro = object.ro;
      ref = object.ref;
      constructor = object.constructor;
    };

    if (force) {
      mod();
    }
    else {
      if (ref) GetTargetObject()->Copy(object);
      else mod();
    }
    return *this;
  }

  shared_ptr<void> Object::Get() {
    if (ref) return GetTargetObject()->Get();
    return ptr;
  }

  string Object::GetTypeId() {
    if (ref) return GetTargetObject()->GetTypeId();
    return type_id;
  }

  Object &Object::SetMethods(string methods) {
    if (ref) return GetTargetObject()->SetMethods(methods);
    this->methods = methods;
    return *this;
  }

  Object &Object::AppendMethod(string method) {
    if (ref) return GetTargetObject()->AppendMethod(method);
    this->methods.append("|" + method);
    return *this;
  }

  Object &Object::SetTokenType(TokenTypeEnum tokenTypeEnum) {
    if (ref) return GetTargetObject()->SetTokenType(tokenTypeEnum);
    this->tokenTypeEnum = tokenTypeEnum;
    return *this;
  }

  Object &Object::SetRo(bool ro) {
    if (ref) return GetTargetObject()->SetRo(ro);
    this->ro = ro;
    return *this;
  }

  string Object::GetMethods() {
    if (ref) return GetTargetObject()->GetMethods();
    return methods;
  }

  TokenTypeEnum Object::GetTokenType() {
    if (ref) return GetTargetObject()->GetTokenType();
    return tokenTypeEnum;
  }

  bool Object::IsRo() {
    if (ref) return GetTargetObject()->IsRo();
    return ro;
  }

  bool Object::ConstructorFlag() {
    bool result = constructor;
    constructor = false;
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