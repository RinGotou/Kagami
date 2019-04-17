#pragma once
#include "common.h"

namespace kagami {
  class Object;
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

  enum ObjectMode {
    kObjectNormal    = 0,
    kObjectRef       = 1,
    kObjectMemberRef = 2
  };

  struct HasherInterface {
    virtual size_t Get(shared_ptr<void>) const = 0;
    virtual ~HasherInterface() {}
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
    ObjectPointer real_dest_;
    ObjectMode mode_;
    bool constructor_;
    int64_t ref_count_;
    shared_ptr<void> ptr_;
    string type_id_;

  public:
    ~Object() {
      if (mode_ == kObjectRef && real_dest_ != nullptr) {
        real_dest_->ref_count_ -= 1;
      }
    }

    Object() :
      real_dest_(nullptr),
      mode_(kObjectNormal),
      constructor_(false),
      ref_count_(0),
      ptr_(nullptr),
      type_id_(kTypeIdNull) {}

    Object(const Object &obj) :
      real_dest_(obj.real_dest_),
      mode_(obj.mode_),
      constructor_(obj.constructor_),
      ref_count_(0),
      ptr_(obj.ptr_),
      type_id_(obj.type_id_) {
      if (obj.mode_ == kObjectRef) {
        real_dest_->ref_count_ += 1;
      }
    }

    Object(const Object &&obj) :
      Object(obj) {}

    template <class T>
    Object(shared_ptr<T> ptr, string type_id) :
      real_dest_(nullptr),
      mode_(kObjectNormal),
      constructor_(false),
      ref_count_(0),
      ptr_(ptr), 
      type_id_(type_id) {}

    template <class T>
    Object(T &t, string type_id) :
      real_dest_(nullptr),
      mode_(kObjectNormal),
      constructor_(false),
      ref_count_(0),
      ptr_(make_shared<T>(t)),
      type_id_(type_id) {}

    template <class T>
    Object(T &&t, string type_id) :
      Object(t, type_id) {}

    Object(string str) :
      real_dest_(nullptr),
      mode_(kObjectNormal),
      constructor_(false),
      ref_count_(0),
      ptr_(std::make_shared<string>(str)),
      type_id_(kTypeIdString) {}

    Object &operator=(const Object &object);
    Object &Manage(shared_ptr<void> ptr, string type_id);
    Object &swap(Object &obj);
    Object &CreateRef(Object &object);

    shared_ptr<void> Get() {
      if (mode_ == kObjectRef) return real_dest_->Get();
      return ptr_;
    }

    template <class Tx>
    Object &CreateMemberRef(Tx &tx, string type_id) {
      type_id_ = type_id;
      mode_ = kObjectMemberRef;

      TargetMember<Tx> target(tx);
      ptr_ = make_shared<TargetMember<Tx>>(target);

      return *this;
    }

    Object &Deref() {
      if (mode_ == kObjectRef) {
        return *real_dest_;
      }

      return *this;
    }

    template <class Tx>
    Tx &Cast() {
      if (mode_ == kObjectRef) { 
        return real_dest_->Cast<Tx>(); 
      }

      if (mode_ == kObjectMemberRef) {
        return static_pointer_cast<TargetMember<Tx>>(ptr_)->Cast();
      }

      return *std::static_pointer_cast<Tx>(ptr_);
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

    bool operator==(const Object &obj) const { 
      if (&obj == this) return true;
      return ptr_ == obj.ptr_; 
    }

    bool operator==(const Object &&obj) const { 
      return ptr_ == obj.ptr_;
    }

    Object &operator=(const Object &&object) { return operator=(object); }

    Object &swap(Object &&obj) { return swap(obj); }

    string GetTypeId() const { return type_id_; }

    int64_t ObjRefCount() const { return ref_count_; }

    bool IsRef() const { return mode_ == kObjectRef; }

    bool IsMemberRef() const { return mode_ == kObjectMemberRef; }

    bool Null() const { return ptr_ == nullptr && real_dest_ == nullptr; }
  };

  using ObjectArray = vector<Object>;
  using ManagedArray = shared_ptr<ObjectArray>;

  class ObjectContainer {
  private:
    ObjectContainer *prev_;
    map<string, Object> base_;
    unordered_map<string, Object *>dest_map_;

    bool CheckObject(string id) {
      return (base_.find(id) != base_.end());
    }

    void BuildCache();
  public:
    bool Add(string id, Object source);
    Object *Find(string id, bool forward_seeking = true);
    string FindDomain(string id, bool forward_seeking = true);
    void ClearExcept(string exceptions);

    ObjectContainer() : prev_(nullptr), base_(), dest_map_() {}

    ObjectContainer(const ObjectContainer &&mgr) {}

    ObjectContainer(const ObjectContainer &container) :
      prev_(container.prev_), base_(container.base_),
      dest_map_(container.dest_map_) {}

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

    ObjectContainer &SetPreviousContainer(ObjectContainer &prev) {
      prev_ = &prev;
      return *this;
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
      ObjectContainer container;
      if (!base_.empty()) {
        container.SetPreviousContainer(base_.back());
      }
      base_.push_back(container);
      return *this;
    }

    ObjectStack &Pop() {
      base_.pop_back();
      return *this;
    }

    DataType &GetBase() {
      return base_;
    }

    void MergeMap(ObjectMap &p);
    Object *Find(string id);
    bool CreateObject(string id, Object obj);
  };
}
