#pragma once
#include "list.h" 
#include "common.h"

namespace kagami {
  class Object;
  class ObjectCopyingPolicy;
  class ObjectContainer;
  class ObjectMap;
  class Message;

  using ObjectPointer = Object * ;
  using Activity = Message(*)(ObjectMap &);
  using NamedObject = pair<string, Object>;
  using CopyingPolicy = shared_ptr<void>(*)(shared_ptr<void>);
  using ContainerPool = kagami::list<ObjectContainer>;

  class Object {
    struct TargetObject { 
      Object *ptr; 
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


    Object &Set(shared_ptr<void> ptr, string type_id) {
      if (ref_) return GetTargetObject()->Set(ptr, type_id);
      ptr_ = ptr;
      type_id_ = type_id;
      return *this;
    }

    shared_ptr<void> Get() {
      if (ref_) return GetTargetObject()->Get();
      return ptr_;
    }

    template <class Tx>
    Tx &Cast() {
      return std::static_pointer_cast<Tx>(ptr_);
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

    Object &Copy(Object &&object) { 
      return this->Copy(object); 
    }

    bool IsRef() const { 
      return ref_; 
    }

    Object &Ref(Object &object);
    Object &Copy(Object &object, bool force = false);
  };

  class ObjectCopyingPolicy {
  private:
    CopyingPolicy solver_;
    string methods_;
  public:
    ObjectCopyingPolicy() : 
      methods_("") { 

      solver_ = nullptr; 
    }
    ObjectCopyingPolicy(CopyingPolicy solver, 
      string methods) : 
      methods_(methods){

      solver_ = solver;
    }

    shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) const {
      shared_ptr<void> result = nullptr;
      if (target != nullptr) {
        result = solver_(target);
      }
      return result;
    }

    string GetMethods() const { 
      return methods_; 
    }
  };

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

    ObjectContainer(ObjectContainer &&mgr) {}

    ObjectContainer(ObjectContainer &container) :base_(container.base_) {}

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

    bool CheckTypeId(string id, string type_id) {
      return this->at(id).GetTypeId() == type_id;
    }

    bool CheckTypeId(string id, ComparingFunction func) {
      return func(this->at(id));
    }

    void Input(string id, Object obj) {
      this->insert(NamedObject(id, obj));
    }

    void Input(string id) {
      this->insert(NamedObject(id, Object()));
    }

    int GetVaSize() {
      int size;
      auto it = this->find(kStrVaSize);
      it != this->end() ?
        size = stoi(*static_pointer_cast<string>(it->second.Get())) :
        size = -1;
      return size;
    }
  };
}
