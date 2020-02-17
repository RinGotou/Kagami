#pragma once
#include "common.h"

namespace kagami {
  class Object;
  class ObjectContainer;
  class ObjectMap;
  class Message;

  using GenericPointer = uintptr_t;
  using ObjectPointer = Object *;
  using ObjectRef = Object &;
  using Comparator = bool(*)(Object &, Object &);
  using NamedObject = pair<string, Object>;
  using DeliveryImpl = shared_ptr<void>(*)(shared_ptr<void>);
  using ContainerPool = list<ObjectContainer>;
  using ExternalMemoryDisposer = void(*)(void *, const char *);
  using MemoryDisposer = void(*)(void *, int);

  vector<string> BuildStringVector(string source);
  string CombineStringVector(vector<string> target);

  enum ObjectMode {
    kObjectNormal    = 1,
    kObjectRef       = 2,
    kObjectExternal  = 3,
    kObjectDelegator = 4    //for language key features
  };

  using HasherFunction = size_t(*)(shared_ptr<void>);

  template <typename T>
  size_t PlainHasher(shared_ptr<void> ptr) {
    auto hasher = std::hash<T>();
    return hasher(*static_pointer_cast<T>(ptr));
  }

  size_t PointerHasher(shared_ptr<void> ptr);

  template <typename T>
  shared_ptr<void> PlainDeliveryImpl(shared_ptr<void> target) {
    T temp(*static_pointer_cast<T>(target));
    return make_shared<T>(temp);
  }

  shared_ptr<void> ShallowDelivery(shared_ptr<void> target);

  class ObjectTraits {
  private:
    DeliveryImpl delivering_impl_;
    Comparator comparator_;
    HasherFunction hasher_;
    vector<string> methods_;

  public:
    ObjectTraits() = delete;

    ObjectTraits(
      DeliveryImpl dlvy,
      string methods,
      HasherFunction hasher = nullptr,
      Comparator comparator = nullptr) :
      delivering_impl_(dlvy),
      comparator_(comparator),
      methods_(BuildStringVector(methods)),
      hasher_(hasher) {}

    vector<string> &GetMethods() { return methods_; }
    HasherFunction GetHasher() { return hasher_; }
    Comparator GetComparator() { return comparator_; }
    DeliveryImpl GetDeliveringImpl() { return delivering_impl_; }
  };

  class ExternalRCContainer {
  protected:
    void *ptr_;
    ExternalMemoryDisposer disposer_;
    string type_id_;

  public:
    ~ExternalRCContainer() {
      if (disposer_ != nullptr) disposer_(ptr_, type_id_.data());
    }
    ExternalRCContainer() = delete;
    ExternalRCContainer(void *ptr, ExternalMemoryDisposer disposer, 
      string type_id) :
      ptr_(ptr), disposer_(disposer), type_id_(type_id) {}
  };

  //TODO:delegator mode
  class Object {
  private:
    void *real_dest_;
    ObjectMode mode_;
    bool delivering_;
    bool sub_container_;
    shared_ptr<void> ptr_;
    string type_id_;

  public:
    ~Object() {}

    Object() : real_dest_(nullptr), mode_(kObjectNormal), delivering_(false),
      sub_container_(false), ptr_(nullptr), type_id_(kTypeIdNull) {}

    Object(const Object &obj) :
      real_dest_(obj.real_dest_), mode_(obj.mode_), delivering_(obj.delivering_),
      sub_container_(false), ptr_(obj.ptr_), type_id_(obj.type_id_) {}

    Object(const Object &&obj) noexcept :
      Object(obj) {}

    template <typename T>
    Object(shared_ptr<T> ptr, string type_id) :
      real_dest_(nullptr), mode_(kObjectNormal), delivering_(false),
      sub_container_(false), ptr_(ptr), type_id_(type_id) {}

    template <typename T>
    Object(T &t, string type_id) :
      real_dest_(nullptr), mode_(kObjectNormal), delivering_(false),
      sub_container_(false), ptr_(make_shared<T>(t)), type_id_(type_id) {}

    template <typename T>
    Object(T &&t, string type_id) :
      Object(t, type_id) {}

    template <typename T>
    Object(T *ptr, string type_id) :
      real_dest_((void *)ptr), mode_(kObjectDelegator),  delivering_(false),
      sub_container_(false), ptr_(nullptr), type_id_(type_id) {}

    Object(void *ext_ptr, ExternalMemoryDisposer disposer, string type_id) :
      real_dest_(ext_ptr), mode_(kObjectExternal), delivering_(false), 
      sub_container_(false), ptr_(make_shared<ExternalRCContainer>(ext_ptr, disposer, type_id)),
      type_id_(type_id) {}

    Object(string str) :
      real_dest_(nullptr), mode_(kObjectNormal), delivering_(false),
      sub_container_(false), ptr_(make_shared<string>(str)), type_id_(kTypeIdString) {}

    Object &operator=(const Object &object);
    Object &PackContent(shared_ptr<void> ptr, string type_id);
    Object &swap(Object &obj);
    Object &PackObject(Object &object);

    shared_ptr<void> Get() {
      if (mode_ == kObjectRef) {
        return static_cast<ObjectPointer>(real_dest_)->Get();
      }

      return ptr_;
    }

    Object &Unpack() {
      if (mode_ == kObjectRef) {
        return *static_cast<ObjectPointer>(real_dest_);
      }
      return *this;
    }

    template <typename Tx>
    Tx &Cast() {
      if (mode_ == kObjectRef) { 
        return static_cast<ObjectPointer>(real_dest_)->Cast<Tx>(); 
      }

      if (mode_ == kObjectDelegator) {
        return *static_cast<Tx *>(real_dest_);
      }

      return *std::static_pointer_cast<Tx>(ptr_);
    }

    Object &SetDeliveringFlag() {
      delivering_ = true;
      return *this;
    }

    Object &RemoveDeliveringFlag() {
      delivering_ = false;
      return *this;
    }

    bool GetDeliveringFlag() {
      if (mode_ == kObjectRef) {
        return static_cast<ObjectPointer>(real_dest_)
          ->GetDeliveringFlag();
      }
      bool result = delivering_;
      delivering_ = false;
      return result;
    }

    bool SeekDeliveringFlag() {
      if (mode_ == kObjectRef) {
        return static_cast<ObjectPointer>(real_dest_)
          ->SeekDeliveringFlag();
      }
      return delivering_;
    }

    bool operator==(const Object &obj) = delete;
    bool operator==(const Object &&obj) = delete;

    Object *GetRealDest() { return static_cast<ObjectPointer>(real_dest_); }
    void *GetExternalPointer() { return real_dest_; }
    Object &operator=(const Object &&object) { return operator=(object); }
    Object &swap(Object &&obj) { return swap(obj); }
    string GetTypeId() const { return type_id_; }
    bool IsRef() const { return mode_ == kObjectRef; }
    bool Null() const { return ptr_ == nullptr && real_dest_ == nullptr; }
    ObjectMode GetMode() const { return mode_; }
    void SetContainerFlag() { sub_container_ = true; }
    bool IsSubContainer() { return sub_container_; }
  };

  using ObjectArray = deque<Object>;
  using ManagedArray = shared_ptr<ObjectArray>;
  using ObjectPair = pair<Object, Object>;
  using ManagedPair = shared_ptr<ObjectPair>;

  class ObjectContainer {
  private:
    ObjectContainer *delegator_;
    ObjectContainer *prev_;
    map<string, Object> base_;
    unordered_map<string, Object *> dest_map_;

    bool IsDelegated() const { 
      return delegator_ != nullptr; 
    }

    bool CheckObject(string id) {
      return (base_.find(id) != base_.end());
    }

    void BuildCache();
  public:
    bool Add(string id, Object source);
    bool Dispose(string id);
    Object *Find(string id, bool forward_seeking = true);
    Object *FindWithDomain(string id, string domain, bool forward_seeking = true);
    bool IsInside(Object *ptr);
    void ClearExcept(string exceptions);

    ObjectContainer() : delegator_(nullptr),
      prev_(nullptr), base_(), dest_map_() {}

    ObjectContainer(const ObjectContainer &&mgr) :
    delegator_(mgr.delegator_), prev_(mgr.prev_) {}

    ObjectContainer(const ObjectContainer &container) :
      delegator_(container.delegator_), prev_(container.prev_) {
      if (!container.base_.empty()) {
        base_ = container.base_;
        BuildCache();
      }
    }

    ObjectContainer(ObjectContainer *delegator) :
      delegator_(delegator), prev_(nullptr) {}

    bool Empty() const {
      return base_.empty();
    }

    ObjectContainer &operator=(ObjectContainer &mgr) {
      if (IsDelegated()) return delegator_->operator=(mgr);

      base_ = mgr.base_;
      return *this;
    }

    void Clear() {
      if (IsDelegated()) delegator_->Clear();

      base_.clear();
      BuildCache();
    }

    map<string, Object> &GetContent() {
      if (IsDelegated()) return delegator_->GetContent();
      return base_;
    }

    unordered_map<string, Object *> &GetHashMap() {
      if (IsDelegated()) return delegator_->GetHashMap();
      return dest_map_;
    }

    ObjectContainer &SetPreviousContainer(ObjectContainer *prev) {
      if (IsDelegated()) return delegator_->SetPreviousContainer(prev);
      prev_ = prev;
      return *this;
    }
  };

  using ObjectStruct = ObjectContainer;

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

    template <typename T>
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
    ObjectContainer *root_container_;
    DataType base_;
    ObjectStack *prev_;
    bool delegated_;

  public:
    ObjectStack() :
      root_container_(nullptr),
      base_(),
      prev_(nullptr),
      delegated_(false) {}

    ObjectStack(const ObjectStack &rhs) :
      root_container_(rhs.root_container_),
      base_(rhs.base_),
      prev_(rhs.prev_),
      delegated_(false) {}

    ObjectStack(const ObjectStack &&rhs) :
      ObjectStack(rhs) {}

    ObjectStack &SetPreviousStack(ObjectStack &prev) {
      prev_ = &prev;
      return *this;
    }

    ObjectStack& SetDelegatedRoot(ObjectContainer& root) {
      if(!base_.empty()) base_.pop_front();
      base_.push_front(ObjectContainer(&root));
      delegated_ = true;
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
      if (base_.size() == 1 && delegated_) {
        delegated_ = false;
        return *this;
      }

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
    Object *Find(string id, string domain);
    bool CreateObject(string id, Object obj);
    bool DisposeObjectInCurrentScope(string id);
    bool DisposeObject(string id);
  };
}
