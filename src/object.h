#pragma once
#include "common.h"
#include "list.h" 

namespace kagami {
  class Object;
  class ObjectPlanner;
  class ObjectContainer;
  class ObjectMap;

  using ObjectPointer = Object * ;
  using ObjectPair = pair<string, Object>;
  using Activity = Message(*)(ObjectMap &);
  using NamedObject = pair<string, Object>;
  using ObjTypeId = string;

  using ContainerPool = kagami::list<ObjectContainer>;

  /*Object Class
  A shared void pointer is packaged in this.Almost all variables and
  constants are managed by shared pointers.This class will be packaged
  in ObjectContainer class.
  */
  class Object {
    using TargetObject = struct { Object *ptr; };
    std::shared_ptr<void> ptr_;
    string type_id_;
    string methods_;
    string domain_;
    TokenTypeEnum token_type_;
    bool ro_, ref_, constructor_;

    ObjectPointer GetTargetObject() { 
      return static_pointer_cast<TargetObject>(ptr_)->ptr; 
    }
  public:
    Object &Ref(Object &object);
    Object &Copy(Object &object, bool force = false);

    Object() :
      ptr_(nullptr),
      type_id_(kStrNull),
      methods_(),
      domain_(),
      ro_(false),
      ref_(false),
      constructor_(false) {

      token_type_ = TokenTypeEnum::T_NUL;
    }

    Object &AppendMethod(string method) {
      if (ref_) return GetTargetObject()->AppendMethod(method);
      methods_.append("|" + method);
      return *this;
    }

    Object &SetTokenType(TokenTypeEnum token_type) {
      if (ref_) return GetTargetObject()->SetTokenType(token_type);
      token_type_ = token_type;
      return *this;
    }

    TokenTypeEnum GetTokenType() {
      if (ref_) return GetTargetObject()->GetTokenType();
      return token_type_;
    }

    Object &set_ro(bool ro) {
      if (ref_) return GetTargetObject()->set_ro(ro);
      ro_ = ro;
      return *this;
    }

    bool get_ro() {
      if (ref_) return GetTargetObject()->get_ro();
      return ro_;
    }

    string GetMethods() {
      if (ref_) return GetTargetObject()->GetMethods();
      return methods_;
    }

    Object &Set(shared_ptr<void> ptr, string type_id) {
      if (ref_) return GetTargetObject()->Set(ptr, type_id);
      ptr_ = ptr;
      type_id_ = type_id;
      return *this;
    }

    Object &Manage(string t, TokenTypeEnum tokenType) {
      if (ref_) return GetTargetObject()->Manage(t, tokenType);
      ptr_ = std::make_shared<string>(t);
      type_id_ = kTypeIdRawString;
      methods_ = kRawStringMethods;
      token_type_ = tokenType;
      return *this;
    }


    Object &Set(shared_ptr<void> ptr, string type_id, string methods, bool ro) {
      if (ref_) return GetTargetObject()->Set(ptr, type_id, methods, ro);
      ptr_ = ptr;
      type_id_ = type_id;
      methods_ = methods;
      ro_ = ro;
      return *this;
    }

    shared_ptr<void> Get() {
      if (ref_) return GetTargetObject()->Get();
      return ptr_;
    }

    void Clear() {
      ptr_ = make_shared<int>(0);
      type_id_ = kTypeIdNull;
      methods_.clear();
      token_type_ = TokenTypeEnum::T_NUL;
      ro_ = false;
      ref_ = false;
    }

    bool Compare(Object &object) const {
      return (ptr_ == object.ptr_ &&
        type_id_ == object.type_id_ &&
        methods_ == object.methods_ &&
        token_type_ == object.token_type_ &&
        ro_ == object.ro_ &&
        constructor_ == object.constructor_ &&
        ref_ == object.ref_);
    }

    Object &SetMethods(string methods) {
      if (ref_) return GetTargetObject()->SetMethods(methods);
      methods_ = methods;
      return *this;
    }

    string GetTypeId() {
      if (ref_) return GetTargetObject()->GetTypeId();
      return type_id_;
    }

    Object &SetConstructorFlag() { 
      constructor_ = true; 
      return *this; 
    }

    bool GetConstructorFlag() {
      bool result = constructor_;
      constructor_ = false;
      return result;
    }

    Object &SetDomain(string domain) {
      domain_ = domain;
      return *this;
    }

    string GetDomain() const {
      return domain_;
    }

    Object &Copy(Object &&object) { 
      return this->Copy(object); 
    }

    bool IsRef() const { 
      return ref_; 
    }

    bool operator==(Object &object) const { 
      return Compare(object); 
    }
    bool operator!=(Object &object) const { 
      return !Compare(object); 
    }
  };

  /* Object Template Class
  this class contains custom class info for script language.
  */
  class ObjectPlanner {
  private:
    CopyCreator creator_;
    string methods_;
  public:
    ObjectPlanner() : 
      methods_(kStrEmpty) { 

      creator_ = nullptr; 
    }
    ObjectPlanner(CopyCreator creator, 
      string methods) : 
      methods_(methods){

      creator_ = creator;
    }

    shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) const {
      shared_ptr<void> result = nullptr;
      if (target != nullptr) {
        result = creator_(target);
      }
      return result;
    }

    string GetMethods() const { 
      return methods_; 
    }
  };

  /*ObjectContainer Class
  MemoryManger will be filled with Object and manage life cycle of variables
  and constants.
  */
  class ObjectContainer {
  private:
    list<NamedObject> base;

    bool CheckObject(string id) {
      for (size_t i = 0; i < base.size(); ++i) {
        NamedObject &object = base.at(i);
        if (object.first == id) return false;
      }
      return true;
    }
  public:
    bool Add(string id, Object &source);
    Object *Find(string id, string domain = kStrEmpty);
    void Dispose(string id, string domain = kStrEmpty);

    ObjectContainer() {}

    ObjectContainer(ObjectContainer &&mgr) {}

    ObjectContainer(ObjectContainer &container) {
      this->base.copy(container.base);
    }

    bool Empty() const { 
      return base.empty(); 
    }

    ObjectContainer &operator=(ObjectContainer &mgr) { 
      base.copy(mgr.base); return *this; 
    }

    void clear() {
      base.clear();
    }
  };

  class ObjectMap : public map<string, Object> {
  protected:
    using ComparingFunction = bool(*)(Object &);
  public:
    bool Search(string id) {
      auto it = this->find(id);
      return it != this->end();
    }

    template <class T>
    T &Get(string id) {
      return *static_pointer_cast<T>(this->at(id).Get());
    }

    Object &operator()(string id) {
      return this->at(id);
    }

    bool CheckTypeId(string id, string type_id) {
      return this->at(id).GetTypeId() == type_id;
    }

    bool CheckTypeId(string id, ComparingFunction func) {
      return func(this->at(id));
    }

    void Input(string id, Object &obj) {
      this->insert(NamedObject(id, obj));
    }

    void Input(string id) {
      this->insert(NamedObject(id, Object()));
    }
  };
}
