#pragma once
#include "common.h"

namespace kagami {
  class Object;
  class ObjectPolicy;
  class ObjectContainer;
  class ObjectMap;
  class Message;

  using ObjectPointer = Object * ;
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

  class Object {
    struct TargetObject { 
      ObjectPointer ptr; 
    };

    std::shared_ptr<void> ptr_;
    string type_id_;
    bool ref_;
    bool constructor_;

    ObjectPointer GetTargetObject() { 
      return static_pointer_cast<TargetObject>(ptr_)->ptr; 
    }

  public:
    Object() :
      ptr_(nullptr),
      type_id_(kTypeIdNull),
      ref_(false),
      constructor_(false) {}

    Object(const Object &obj) :
      ptr_(obj.ptr_),
      type_id_(obj.type_id_),
      ref_(obj.ref_),
      constructor_(obj.constructor_) {}

    Object(const Object &&obj) :
      Object(obj) {}

    Object(shared_ptr<void> ptr, string type_id) :
      ptr_(ptr), 
      type_id_(type_id), 
      ref_(false), 
      constructor_(false) {}

    Object(string str) :
      ptr_(std::make_shared<string>(str)),
      type_id_(kTypeIdRawString),
      ref_(false),
      constructor_(false) {}

    Object &operator=(const Object &object) {
      ptr_ = object.ptr_;
      type_id_ = object.type_id_;
      ref_ = object.ref_;
      constructor_ = object.constructor_;
      return *this;
    }

    Object &operator=(const Object &&object) {
      return this->operator=(object);
    }

    Object &ManageContent(shared_ptr<void> ptr, string type_id) {
      if (ref_) return GetTargetObject()->ManageContent(ptr, type_id);
      ptr_ = ptr;
      type_id_ = type_id;
      return *this;
    }

    shared_ptr<void> Get() {
      if (ref_) return GetTargetObject()->Get();
      return ptr_;
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
      if (ref_) return GetTargetObject()->Cast<Tx>();
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

    Object &CreateRef(Object &object);
    Object &CloneFrom(Object &object, bool force = false);
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
    void Dispose(string id);

    ObjectContainer() {}

    ObjectContainer(const ObjectContainer &&mgr) {}

    ObjectContainer(const ObjectContainer &container) :base_(container.base_) {}

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

    ObjectMap(const std::initializer_list<NamedObject> &&rhs) {
      for (const auto &unit : rhs) {
        this->insert(unit);
      }
    }

    ObjectMap(const map<string, Object> &rhs) :
      map<string, Object>(rhs) {}

    ObjectMap(const map<string, Object> &&rhs) :
      map<string, Object>(rhs) {}

    ObjectMap &operator=(const std::initializer_list<NamedObject> &&rhs){
      for (const auto &unit : rhs) {
        this->insert(unit);
      }

      return *this;
    }

    ObjectMap &operator=(const ObjectMap &rhs) {
      for (const auto &unit : rhs) {
        this->insert(unit);
      }
      return *this;
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

    void merge(ObjectMap &source) {
      for (auto &unit : source) {
        auto it = find(unit.first);
        if (it != end()) {
          it->second = unit.second;
	      }
	      else {
	        this->insert(unit);
	      }
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

    void MergeMap(ObjectMap p) {
      if (p.empty()) return;

      auto &container = base_.back();
      for (auto &unit : p) {
        container.Add(unit.first, unit.second);
      }
    }

    DataType &GetBase() {
      return base_;
    }

    Object *Find(string id);
    bool CreateObject(string id, Object obj);
  };
}
