#pragma once
#include "common.h"

namespace kagami {
  class Object;
  class ObjectContainer;
  class ObjectMap;
  class Message;

  struct _ObjectCommonBase {
    virtual bool IsObjectView() const = 0;
    virtual bool IsAlive() const = 0;
    virtual ~_ObjectCommonBase() {}
  };

  using GenericPointer = uintptr_t;
  using ObjectPointer = Object *;
  using ObjectCommonSlot = _ObjectCommonBase *;
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

  struct ObjectInfo {
    void *real_dest;
    ObjectMode mode;
    bool delivering;
    bool sub_container;
    bool alive;
    string type_id;
  };

  // No use in this time...
  struct ViewCounter {
    bool dead;
    std::atomic_int32_t count;
  };

  using ReferenceLinks = unordered_set<ObjectPointer>;

  class Object : public shared_ptr<void>, virtual public _ObjectCommonBase {
  private:
    ObjectInfo info_;
    ViewCounter *counter_;
    optional<ReferenceLinks> links_;
    mutex gate_;

  private:
    void EraseRefLink() {
      if (info_.mode == kObjectRef && info_.alive) {
        auto *obj = static_cast<ObjectPointer>(info_.real_dest);
        obj->links_.value().erase(this);
      }
    }

    void EstablishRefLink() {
      if (info_.mode == kObjectRef && info_.alive) {
        auto *obj = static_cast<ObjectPointer>(info_.real_dest);
        if(!obj->links_.has_value()) obj->links_.emplace(ReferenceLinks());
        obj->links_.value().insert(this);
      }
    }

  public:
    ~Object() {
      EraseRefLink();

      if (info_.mode != kObjectRef && links_.has_value() && !links_.value().empty()) {
        for (auto &unit : links_.value()) {
          if (unit != nullptr) {
            unit->info_.alive = false;
            unit->info_.real_dest = nullptr;
          }
        }
      }
    }

    Object() : info_{ nullptr, kObjectNormal, false, false, true, kTypeIdNull},
      links_(), shared_ptr<void>(nullptr) {}

    Object(const Object &obj) : 
      info_(obj.info_), links_(std::nullopt), shared_ptr<void>(obj) {
      EstablishRefLink();
    }

    Object(const Object &&obj) noexcept :
      info_(std::move(obj.info_)), links_(std::nullopt), shared_ptr<void>(std::move(obj)) {
      EstablishRefLink();
    }

    template <typename T>
    Object(shared_ptr<T> ptr, string type_id) :
      info_{nullptr, kObjectNormal, false, type_id == kTypeIdStruct, true, type_id},
      links_(), shared_ptr<void>(ptr) {}

    template <typename T>
    Object(T &t, string type_id) :
      info_{nullptr, kObjectNormal, false, type_id == kTypeIdStruct, true, type_id},
      links_(), 
      shared_ptr<void>(make_shared<T>(t)) {}

    template <typename T>
    Object(T &&t, string type_id) :
      info_{ nullptr, kObjectNormal, false, type_id == kTypeIdStruct, true, type_id },
      links_(),
      shared_ptr<void>(make_shared<T>(std::forward<T>(t))) {}

    template <typename T>
    Object(T *ptr, string type_id) :
      info_{(void *)ptr, kObjectDelegator, false, type_id == kTypeIdStruct, true, type_id},
      links_(), shared_ptr<void>(nullptr) {}

    Object(void *ext_ptr, ExternalMemoryDisposer disposer, string type_id) :
      info_{ext_ptr, kObjectExternal, false, false, true, type_id}, links_(std::nullopt),
      shared_ptr<void>(make_shared<ExternalRCContainer>(ext_ptr, disposer, type_id)) {}

    Object(string str) :
      info_{nullptr, kObjectNormal, false, false, true, kTypeIdString},
      links_(), shared_ptr<void>(make_shared<string>(str)) {}

    Object(const ObjectInfo &info, const shared_ptr<void> &ptr) :
      info_(info), links_(std::nullopt), shared_ptr<void>(ptr) {
      EstablishRefLink();
    }

    Object &operator=(const Object &object);
    Object &PackContent(shared_ptr<void> ptr, string type_id);
    Object &swap(Object &obj);
    Object &PackObject(Object &object);

    bool IsObjectView() const override { return false; }

    void Impact(ObjectInfo &&info, shared_ptr<void> ptr) {
      info_ = info;
      dynamic_cast<shared_ptr<void> *>(this)->operator=(ptr);
    }

    shared_ptr<void> Get() {
      if (info_.mode == kObjectRef) {
        return static_cast<ObjectPointer>(info_.real_dest)->Get();
      }
      
      return *dynamic_cast<shared_ptr<void> *>(this);
    }

    Object &Unpack() {
      if (info_.mode == kObjectRef) {
        return *static_cast<ObjectPointer>(info_.real_dest);
      }
      return *this;
    }

    template <typename Tx>
    Tx &Cast() {
      if (info_.mode == kObjectRef) { 
        return static_cast<ObjectPointer>(info_.real_dest)->Cast<Tx>(); 
      }

      if (info_.mode == kObjectDelegator) {
        return *static_cast<Tx *>(info_.real_dest);
      }

      return *static_cast<Tx *>(get());
    }

    Object &SetDeliveringFlag() {
      info_.delivering = true;
      return *this;
    }

    Object &RemoveDeliveringFlag() {
      info_.delivering = false;
      return *this;
    }

    bool GetDeliveringFlag() {
      if (info_.mode == kObjectRef) {
        return static_cast<ObjectPointer>(info_.real_dest)
          ->GetDeliveringFlag();
      }
      bool result = info_.delivering;
      info_.delivering = false;
      return result;
    }

    bool SeekDeliveringFlag() {
      if (info_.mode == kObjectRef) {
        return static_cast<ObjectPointer>(info_.real_dest)
          ->SeekDeliveringFlag();
      }
      return info_.delivering;
    }

    bool IsSubContainer() {
      if (info_.mode == kObjectRef) {
        return static_cast<ObjectPointer>(info_.real_dest)->IsSubContainer();
      }

      return info_.sub_container;
    }

    bool operator==(const Object &obj) = delete;
    bool operator==(const Object &&obj) = delete;

    mutex &GetMutex() { return gate_; }
    Object *GetRealDest() { return static_cast<ObjectPointer>(info_.real_dest); }
    ObjectInfo &GetObjectInfoTable() { return info_; }
    void *GetExternalPointer() { return info_.real_dest; }
    Object &operator=(const Object &&object) { return operator=(object); }
    Object &swap(Object &&obj) { return swap(obj); }
    string GetTypeId() const { return info_.type_id; }
    bool IsRef() const { return info_.mode == kObjectRef; }
    bool Null() const { return !this->operator bool() && info_.real_dest == nullptr; }
    ObjectMode GetMode() const { return info_.mode; }
    void SetContainerFlag() { info_.sub_container = true; }
    bool IsAlive() const override { return info_.alive; }
  };

  using MovableObject = unique_ptr<Object>;

  enum class ObjectViewSource {
    kSourceReference,
    kSourceLiteral,
    kSourceNull
  };

  class ObjectView : virtual public _ObjectCommonBase {
  protected:
    using Value = ObjectPointer;
    using Source = ObjectViewSource;

  protected:
    Value value_;

  public:
    Source source;

  public:
    ObjectView() : value_(nullptr) {}
    ObjectView(const ObjectView &rhs) = default;
    ObjectView(const ObjectView &&rhs) : ObjectView(rhs) {}
    ObjectView(ObjectPointer ptr) : value_(ptr) {}

    void operator=(const ObjectView &rhs) { value_ = rhs.value_; }

    bool IsObjectView() const override { return true; }
    void operator=(const ObjectView &&rhs) { operator=(rhs); }
    constexpr Object &Seek() { return *value_; }
    bool IsAlive() const override { return value_->IsAlive(); }
    bool IsValid() const { return value_ != nullptr; }
    Object Dump() { return Seek(); }
  };

  using ObjectArray = deque<Object>;
  using ManagedArray = shared_ptr<ObjectArray>;
  using ObjectPair = pair<Object, Object>;
  using ManagedPair = shared_ptr<ObjectPair>;
  using ObjectCache = pair<string, ObjectPointer>;

  class ObjectContainer {
  private:
    ObjectContainer *delegator_;
    ObjectContainer *prev_;
    unordered_map<string, Object> container_;

    bool IsDelegated() const { 
      return delegator_ != nullptr; 
    }
  public:
    bool Add(string id, Object &source);
    bool Add(string id, Object &&source);
    void Replace(string id, Object &source);
    void Replace(string id, Object &&source);
    Object *Find(const string &id, bool forward_seeking = true);
    Object *FindWithDomain(const string &id, const string &domain, bool forward_seeking = true);
    bool IsInside(Object *ptr);
    void ClearExcept(string exceptions);

    ObjectContainer() : delegator_(nullptr),
      prev_(nullptr), container_() {}

    ObjectContainer(const ObjectContainer &&rhs) :
    delegator_(rhs.delegator_), prev_(rhs.prev_), container_() {}

    ObjectContainer(const ObjectContainer &rhs) :
      delegator_(rhs.delegator_), prev_(rhs.prev_) {
      if (!rhs.container_.empty()) { container_ = rhs.container_; }
    }

    bool Empty() const {
      return container_.empty();
    }

    void Clear() {
      if (IsDelegated()) delegator_->Clear();
      container_.clear();
    }

    unordered_map<string, Object> &GetContent() {
      if (IsDelegated()) return delegator_->GetContent();
      return container_;
    }

    ObjectContainer &SetPreviousContainer(ObjectContainer *prev) {
      if (IsDelegated()) return delegator_->SetPreviousContainer(prev);
      prev_ = prev;
      return *this;
    }

    ObjectContainer &SetDelegatedContainer(ObjectContainer *dest) {
      delegator_ = dest;
      return *this;
    }
  };

  using ObjectStruct = ObjectContainer;

  class ObjectMap : public unordered_map<string, Object> {
  public:
    using ComparingFunction = bool(*)(Object &);

  public:
    ObjectMap() {}

    ObjectMap(const ObjectMap &rhs) :
      unordered_map<string, Object>(rhs) {}

    ObjectMap(const ObjectMap &&rhs) :
      unordered_map<string, Object>(rhs) {}

    ObjectMap(const initializer_list<NamedObject> &rhs) {
      this->clear();
      for (const auto &unit : rhs) {
        this->insert(unit);
      }
    }

    ObjectMap(const initializer_list<NamedObject> &&rhs) :
      ObjectMap(rhs) {}

    ObjectMap(const unordered_map<string, Object> &rhs) :
      unordered_map<string, Object>(rhs) {}

    ObjectMap(const unordered_map<string, Object> &&rhs) :
      unordered_map<string, Object>(rhs) {}

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

  // first - created, second - inherit_last_scope
  using CreationInfo = pair<bool, bool>;

  class ObjectStack {
  private:
    using DataType = list<ObjectContainer>;
    ObjectContainer *root_container_;
    DataType base_;
    ObjectStack *prev_;
    // For delaying scope creation
    stack<CreationInfo, list<CreationInfo>> creation_info_;
    // One-time trigger for avoiding confliction of global scope creation
    bool delegated_;

  private:
    void ScopeCreation(bool inherit_last_scope) {
      // previous container
      auto *prev = base_.empty() ? nullptr : &base_.back();
      // global(base) scope
      auto *base_scope = base_.empty() ? nullptr : &base_.front();
      base_.emplace_back(ObjectContainer());
      // link previous container
      base_.back().SetPreviousContainer(inherit_last_scope ? prev : base_scope);
    }

    bool HasDelayedCreation() {
      if (creation_info_.empty()) return false;
      return !creation_info_.top().first;
    }

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
      // TODO: HasDelayedCreation()?
      if(!base_.empty()) base_.pop_front();
      base_.push_front(ObjectContainer().SetDelegatedContainer(&root));
      delegated_ = true;
      return *this;
    }

    ObjectContainer &GetCurrent() { 
      if (HasDelayedCreation()) ScopeCreation(creation_info_.top().second);
      return base_.back(); 
    }

    bool ClearCurrent() {
      if (!HasDelayedCreation()) {
        if (base_.empty()) return false;
        base_.back().Clear();
      }
      return true;
    }

    bool ClearCurrentExcept(string exceptions) {
      if (!HasDelayedCreation()) {
        if (base_.empty()) return false;
        base_.back().ClearExcept(exceptions);
      }
      return true;
    }

    ObjectStack &Push(bool inherit_last_scope = false, bool delay_creation = false) {
      // if there's a delegated global scope and VM send a scope creation request,
      // just ignore it and destroy trigger
      if (base_.size() == 1 && delegated_) {
        delegated_ = false;
        return *this;
      }

      if (delay_creation) {
        creation_info_.push(CreationInfo(false, inherit_last_scope));
      }
      else {
        if (HasDelayedCreation()) {
          ScopeCreation(creation_info_.top().second);
          creation_info_.top().first = true;
        }
        ScopeCreation(inherit_last_scope);
        creation_info_.push(CreationInfo(true, inherit_last_scope));
      }
      return *this;
    }

    ObjectStack &Pop() {
      if (!HasDelayedCreation()) {
        base_.pop_back();
      }
      if(!creation_info_.empty()) creation_info_.pop();
      return *this;
    }

    DataType &GetBase() {
      if (HasDelayedCreation()) ScopeCreation(creation_info_.top().second);
      return base_;
    }

    void MergeMap(ObjectMap &p);
    Object *Find(const string &id);
    Object *Find(const string &id, const string &domain);
    bool CreateObject(string id, Object &obj);
    bool CreateObject(string id, Object &&obj);
  };
}
