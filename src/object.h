#pragma once
#include "common.h"

namespace kagami {
  class Object;
  class ObjectContainer;
  class ObjectMap;
  class Message;

  using ObjectPointer = Object *;
  using ObjectRef = Object &;
  using Comparator = bool(*)(Object &, Object &);
  using Activity = Message(*)(ObjectMap &);
  using NamedObject = pair<string, Object>;
  using DeliveryImpl = shared_ptr<void>(*)(shared_ptr<void>);
  using ContainerPool = list<ObjectContainer>;
  
  vector<string> BuildStringVector(string source);

  enum ObjectMode {
    kObjectNormal    = 1,
    kObjectRef       = 2,
  };

  using HasherFunction = size_t(*)(shared_ptr<void>);

  template <class T>
  size_t PlainHasher(shared_ptr<void> ptr) {
    auto hasher = std::hash<T>();
    return hasher(*static_pointer_cast<T>(ptr));
  }

  size_t PointerHasher(shared_ptr<void> ptr);

  template <class T>
  shared_ptr<void> PlainDeliveryImpl(shared_ptr<void> target) {
    T temp(*static_pointer_cast<T>(target));
    return make_shared<T>(temp);
  }

  shared_ptr<void> ShallowDelivery(shared_ptr<void> target);

  class ObjectTraits {
  private:
    DeliveryImpl dlvy_;
    Comparator comparator_;
    HasherFunction hasher_;
    vector<string> methods_;

  public:
    ObjectTraits() = delete;

    ObjectTraits(
      DeliveryImpl copying_policy,
      string methods,
      HasherFunction hasher = nullptr,
      Comparator comparator = nullptr) :
      dlvy_(copying_policy),
      comparator_(comparator),
      methods_(BuildStringVector(methods)),
      hasher_(hasher) {}

    vector<string> &GetMethods() { return methods_; }
    HasherFunction GetHasher() { return hasher_; }
    Comparator GetComparator() { return comparator_; }
    DeliveryImpl GetDeliver() { return dlvy_; }
  };

  class Object {
  private:
    ObjectPointer real_dest_;
    ObjectMode mode_;
    bool do_not_copy_;
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
      do_not_copy_(false),
      ref_count_(0),
      ptr_(nullptr),
      type_id_(kTypeIdNull) {}

    Object(const Object &obj) :
      real_dest_(obj.real_dest_),
      mode_(obj.mode_),
      do_not_copy_(obj.do_not_copy_),
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
      do_not_copy_(false),
      ref_count_(0),
      ptr_(ptr), 
      type_id_(type_id) {}

    template <class T>
    Object(T &t, string type_id) :
      real_dest_(nullptr),
      mode_(kObjectNormal),
      do_not_copy_(false),
      ref_count_(0),
      ptr_(make_shared<T>(t)),
      type_id_(type_id) {}

    template <class T>
    Object(T &&t, string type_id) :
      Object(t, type_id) {}

    Object(string str) :
      real_dest_(nullptr),
      mode_(kObjectNormal),
      do_not_copy_(false),
      ref_count_(0),
      ptr_(std::make_shared<string>(str)),
      type_id_(kTypeIdString) {}

    Object &operator=(const Object &object);
    Object &PackContent(shared_ptr<void> ptr, string type_id);
    Object &swap(Object &obj);
    Object &PackObject(Object &object);

    shared_ptr<void> Get() {
      if (mode_ == kObjectRef) return real_dest_->Get();
      return ptr_;
    }

    Object &Unpack() {
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

      return *std::static_pointer_cast<Tx>(ptr_);
    }

    Object &SetDeliverFlag() {
      do_not_copy_ = true;
      return *this;
    }

    Object &RemoveDeliverFlag() {
      do_not_copy_ = false;
      return *this;
    }

    bool GetDeliverFlag() {
      if (mode_ == kObjectRef) {
        return real_dest_->GetDeliverFlag();
      }
      bool result = do_not_copy_;
      do_not_copy_ = false;
      return result;
    }

    bool SeekDeliverFlag() {
      if (mode_ == kObjectRef) {
        return real_dest_->SeekDeliverFlag();
      }
      return do_not_copy_;
    }

    bool operator==(const Object &obj) = delete;
    bool operator==(const Object &&obj) = delete;

    Object *GetRealDest() { return real_dest_; }

    Object &operator=(const Object &&object) { return operator=(object); }

    Object &swap(Object &&obj) { return swap(obj); }

    string GetTypeId() const { return type_id_; }

    int64_t ObjRefCount() const { return ref_count_; }

    bool IsRef() const { return mode_ == kObjectRef; }

    bool Null() const { return ptr_ == nullptr && real_dest_ == nullptr; }
  };

  using ObjectArray = deque<Object>;
  using ManagedArray = shared_ptr<ObjectArray>;
  using ObjectPair = pair<Object, Object>;
  using ManagedPair = shared_ptr<ObjectPair>;

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
    bool Dispose(string id);
    Object *Find(string id, bool forward_seeking = true);
    bool FindDest(Object *ptr);
    string FindDomain(string id, bool forward_seeking = true);
    void ClearExcept(string exceptions);

    ObjectContainer() : prev_(nullptr), base_(), dest_map_() {}

    ObjectContainer(const ObjectContainer &&mgr) {}

    ObjectContainer(const ObjectContainer &container) :
      prev_(container.prev_) {
      if (!container.base_.empty()) {
        base_ = container.base_;
        BuildCache();
      }
    }

    bool Empty() const {
      return base_.empty();
    }

    ObjectContainer &operator=(ObjectContainer &mgr) {
      base_ = mgr.base_;
      return *this;
    }

    void Clear() {
      base_.clear();
      BuildCache();
    }

    auto &GetContent() {
      return base_;
    }

    auto &GetHashMap() {
      return dest_map_;
    }

    ObjectContainer &SetPreviousContainer(ObjectContainer *prev) {
      prev_ = prev;
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
    void Naturalize(ObjectContainer &container);

    ObjectMap &operator=(const initializer_list<NamedObject> &&rhs) {
      return operator=(rhs);
    }

    template <class T>
    T &Cast(string id) {
      return this->operator[](id).Cast<T>();
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

    bool ClearCurrent() {
      if (base_.empty()) return false;
      base_.back().Clear();
      return true;
    }

    bool ClearCurrentExcept(string exceptions) {
      if (base_.empty()) return false;
      base_.back().ClearExcept(exceptions);
      return true;
    }

    ObjectStack &Push() {
      auto *prev = base_.empty() ? nullptr : &base_.back();
      base_.emplace_back(ObjectContainer());
      base_.back().SetPreviousContainer(prev);
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
    bool DisposeObjectInCurrentScope(string id);
  };
}
