#pragma once
#include "common.h"

namespace kagami {
  class Object;
  class ObjectPolicy;
  class ObjectContainer;
  class ObjectMap;
  class Message;

  using ObjectPointer = Object *;
  using ObjectRef = Object &;
  using Activity = Message(*)(ObjectMap &);
  using NamedObject = pair<string, Object>;
  using CopyingPolicy = shared_ptr<void>(*)(shared_ptr<void>);
  using ContainerPool = list<ObjectContainer>;

  vector<string> BuildStringVector(string source);
  
  class ObjectPolicy {
  private:
    CopyingPolicy copying_policy_;
    vector<string> methods_;

  public:
    ObjectPolicy() :
      copying_policy_(nullptr),
      methods_() {}

    ObjectPolicy(CopyingPolicy copying_policy, string methods) :
      copying_policy_(copying_policy),
      methods_(BuildStringVector(methods)) {}

    shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) const {
      shared_ptr<void> result = nullptr;
      if (target != nullptr) {
        result = copying_policy_(target);
      }
      return result;
    }

    vector<string> GetMethods() const {
      return methods_;
    }
  };

  struct TargetObject {
    ObjectPointer ptr;
  };

  template <class Type>
  class TargetMember {
  private:
    Type *dest_;

  public:
    TargetMember() = delete;

    TargetMember(Type &t) : dest_(&t) {}

    TargetMember(const TargetMember &rhs) : dest_(rhs.dest_) {}

    TargetMember(const TargetMember &&rhs) : TargetMember(rhs) {}

    Type &Cast() { return *dest_; }
  };


  class Object {
  private:
    long ref_count_;
    std::shared_ptr<void> ptr_;
    string type_id_;
    bool ref_;
    bool mem_ref_;
    bool constructor_;

    ObjectPointer GetTargetObject() { 
      return static_pointer_cast<TargetObject>(ptr_)->ptr; 
    }

  public:
    ~Object() {
      if (ref_) {
        GetTargetObject()->ref_count_ -= 1;
      }
    }

    Object() :
      ref_count_(0),
      ptr_(nullptr),
      type_id_(kTypeIdNull),
      ref_(false),
      mem_ref_(false),
      constructor_(false) {}

    Object(const Object &obj) :
      ref_count_(0),
      ptr_(obj.ptr_),
      type_id_(obj.type_id_),
      ref_(obj.ref_),
      mem_ref_(false),
      constructor_(obj.constructor_) {
      if (obj.ref_) {
        GetTargetObject()->ref_count_ += 1;
      }
    }

    Object(const Object &&obj) :
      Object(obj) {}

    Object(shared_ptr<void> ptr, string type_id) :
      ref_count_(0),
      ptr_(ptr), 
      type_id_(type_id), 
      ref_(false), 
      mem_ref_(false),
      constructor_(false) {}

    Object(string str) :
      ref_count_(0),
      ptr_(std::make_shared<string>(str)),
      type_id_(kTypeIdString),
      ref_(false),
      mem_ref_(false),
      constructor_(false) {}

    Object &operator=(const Object &object);
    Object &ManageContent(shared_ptr<void> ptr, string type_id);
    Object &swap(Object &obj);
    Object &CreateRef(Object &object);
    Object &CloneFrom(Object &object, bool force = false);

    Object &operator=(const Object &&object) { return operator=(object); }

    Object &swap(Object &&obj) { return swap(obj); }

    shared_ptr<void> Get() {
      if (ref_) return GetTargetObject()->Get();
      return ptr_;
    }

    template <class Tx>
    Object &CreateMemberRef(Tx &tx, string type_id) {
      type_id_ = type_id;
      mem_ref_ = true;

      TargetMember<Tx> target(tx);
      ptr_ = make_shared<TargetMember<Tx>>(target);

      return *this;
    }

    template <class Tx>
    shared_ptr<Tx> Convert() {
      if (ref_) return GetTargetObject()->Convert<Tx>();
      return static_pointer_cast<Tx>(ptr_);
    }

    Object &Deref() {
      if (ref_) return *GetTargetObject();
      return *this;
    }

    template <class Tx>
    Tx &Cast() {
      if (ref_) { 
        return GetTargetObject()->Cast<Tx>(); 
      }

      if (mem_ref_) {
        return static_cast<TargetMember<Tx>>(ptr_)->Cast();
      }

      return *std::static_pointer_cast<Tx>(ptr_);
    }

    string GetTypeId() {
      if (ref_) return GetTargetObject()->GetTypeId();
      return type_id_;
    }

    Object &SetConstructorFlag() {
      constructor_ = true;
      return *this;
    }

    long use_count() {
      if (ref_) return GetTargetObject()->use_count();
      return ptr_.use_count();
    }

    long ObjRefCount() const {
      return ref_count_;
    }

    bool GetConstructorFlag() {
      bool result = constructor_;
      constructor_ = false;
      return result;
    }

    Object &CloneFrom(Object &&object) {
      return this->CloneFrom(object);
    }

    bool IsRef() const {
      return ref_;
    }

    bool Null() const {
      return ptr_ == nullptr;
    }

    long Count() const {
      return ptr_.use_count();
    }
  };

  using ObjectArray = vector<Object>;

  class ObjectContainer {
  private:
    map<string, Object> base_;

    bool CheckObject(string id) {
      return (base_.find(id) != base_.end());
    }
  public:
    bool Add(string id, Object source);
    Object *Find(string id);
    void ClearExcept(string exceptions);

    ObjectContainer() {}

    ObjectContainer(const ObjectContainer &&mgr) {}

    ObjectContainer(const ObjectContainer &container) :
      base_(container.base_) {}

    bool Empty() const {
      return base_.empty();
    }

    ObjectContainer &operator=(ObjectContainer &mgr) {
      base_ = mgr.base_;
      return *this;
    }

    void clear() {
      base_.clear();
    }

    map<string, Object> &GetContent() {
      return base_;
    }
  };

  class ObjectMap : public map<string, Object> {
  public:
    using ComparingFunction = bool(*)(Object &);

  public:
    ObjectMap() {}

    ObjectMap(const ObjectMap &rhs) :
      map<string, Object>(rhs) {}

    ObjectMap(const ObjectMap &&rhs) :
      map<string, Object>(rhs) {}

    ObjectMap(const initializer_list<NamedObject> &rhs) {
      this->clear();
      for (const auto &unit : rhs) {
        this->insert(unit);
      }
    }

    ObjectMap(const initializer_list<NamedObject> &&rhs) :
      ObjectMap(rhs) {}

    ObjectMap(const map<string, Object> &rhs) :
      map<string, Object>(rhs) {}

    ObjectMap(const map<string, Object> &&rhs) :
      map<string, Object>(rhs) {}

    ObjectMap &operator=(const initializer_list<NamedObject> &rhs);
    ObjectMap &operator=(const ObjectMap &rhs);
    void merge(ObjectMap &source);

    ObjectMap &operator=(const initializer_list<NamedObject> &&rhs) {
      return operator=(rhs);
    }

    template <class T>
    T &Cast(string id) {
      return this->at(id).Cast<T>();
    }

    auto insert_pair(string id, Object obj) {
      return this->insert(NamedObject(id, obj));
    }

    bool CheckTypeId(string id, string type_id) {
      return this->at(id).GetTypeId() == type_id;
    }

    bool CheckTypeId(string id, ComparingFunction func) {
      return func(this->at(id));
    }

    void dispose(string id) {
      auto it = find(id);

      if (it != end()) {
        erase(it);
      }
    }
  };

  class ObjectStack {
  private:
    using DataType = list<ObjectContainer>;
    DataType base_;
    ObjectStack *prev_;

  public:
    ObjectStack() :
      base_(),
      prev_(nullptr) {}

    ObjectStack(const ObjectStack &rhs) :
      base_(rhs.base_),
      prev_(rhs.prev_) {}

    ObjectStack(const ObjectStack &&rhs) :
      ObjectStack(rhs) {}

    ObjectStack &SetPreviousStack(ObjectStack &prev) {
      prev_ = &prev;
      return *this;
    }

    ObjectContainer &GetCurrent() { return base_.back(); }

    ObjectStack &Push() {
      base_.push_back(ObjectContainer());
      return *this;
    }

    ObjectStack &Pop() {
      base_.pop_back();
      return *this;
    }

    DataType &GetBase() {
      return base_;
    }

    void MergeMap(ObjectMap p);
    Object *Find(string id);
    bool CreateObject(string id, Object obj);
  };
}
