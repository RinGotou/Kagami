#pragma once
#include "interface.h"

namespace kagami::management {
  using InterfaceCollection = map<string, Interface>;
  using InterfaceHashMap = unordered_map<string, Interface *>;

  void CreateNewInterface(Interface interface);
  Interface *FindInterface(string id, string domain = kTypeIdNull);

  Object *CreateConstantObject(string id, Object &object);
  Object *CreateConstantObject(string id, Object &&object);
  Object GetConstantObject(string id);
}

namespace kagami::management::type {
  using HasherFunction = size_t(*)(shared_ptr<void>);

  template <class T>
  struct PlainHasher : public HasherInterface {
    size_t Get(shared_ptr<void> ptr) const override {
      auto hasher = std::hash<T>();
      return hasher(*static_pointer_cast<T>(ptr));
    }
  };


  template <class T, HasherFunction hash_func>
  struct CustomHasher : public HasherInterface {
    size_t Get(shared_ptr<void> ptr) const override {
      return hash_func(ptr);
    }
  };

  /* Hasher for object using FakeCopy() */
  struct PointerHasher : public HasherInterface {
    size_t Get(shared_ptr<void> ptr) const override {
      auto hasher = std::hash<shared_ptr<void>>();
      return hasher(ptr);
    }
  };

  template <class T>
  bool PlainComparator(Object &lhs, Object &rhs) {
    return lhs.Cast<T>() == rhs.Cast<T>();
  }

  class ObjectPolicy {
  private:
    CopyingPolicy copying_policy_;
    ObjectComparator comparator_;
    shared_ptr<HasherInterface> hasher_;
    vector<string> methods_;

  public:
    ObjectPolicy() = delete;

    ObjectPolicy(
      CopyingPolicy copying_policy,
      string methods,
      shared_ptr<HasherInterface> hasher = nullptr,
      ObjectComparator comparator = nullptr) :
      copying_policy_(copying_policy),
      comparator_(comparator),
      methods_(BuildStringVector(methods)),
      hasher_(std::move(hasher)) {}

    shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) const;

    vector<string> &GetMethods() { return methods_;}
    shared_ptr<HasherInterface> &GetHasher() { return hasher_; }
    ObjectComparator GetComparator() { return comparator_; }
  };

  vector<string> GetMethods(string id);
  bool CheckMethod(string func_id, string domain);
  size_t GetHash(Object &obj);
  bool IsHashable(Object &obj);
  void NewType(string id, ObjectPolicy temp);
  Object CreateObjectCopy(Object &object);
  bool CheckBehavior(Object obj, string method_str);
  bool CompareObjects(Object &lhs, Object &rhs);

  class NewTypeSetup {
  private:
    string type_name_;
    string methods_;
    CopyingPolicy policy_;
    ObjectComparator comparator_;
    shared_ptr<HasherInterface> hasher_;
    vector<Interface> interfaces_;
    Interface constructor_;

  public:
    NewTypeSetup() = delete;

    template <class HasherType>
    NewTypeSetup(
      string type_name,
      CopyingPolicy policy,
      HasherType hasher) :
      type_name_(type_name),
      policy_(policy),
      comparator_(nullptr),
      hasher_(new HasherType(hasher)) {
      static_assert(is_base_of<HasherInterface, HasherType>::value,
        "Wrong hasher type.");
    }

    NewTypeSetup(string type_name, CopyingPolicy policy) :
      type_name_(type_name), policy_(policy), hasher_(nullptr) {}

    NewTypeSetup &InitConstructor(Interface interface) {
      constructor_ = interface; return *this; 
    }

    NewTypeSetup &InitComparator(ObjectComparator comparator) {
      comparator_ = comparator; return *this; 
    }

    NewTypeSetup &InitMethods(initializer_list<Interface> &&rhs);
    ~NewTypeSetup();
  };
}

namespace std {
  template <>
  struct hash<kagami::Object> {
    size_t operator()(kagami::Object const &rhs) const {
      auto copy = rhs; //solve with limitation
      size_t value = 0;
      if (kagami::management::type::IsHashable(copy)) {
        value = kagami::management::type::GetHash(copy);
      }

      return value;
    }
  };

  template <>
  struct equal_to<kagami::Object> {
    bool operator()(kagami::Object const &lhs, kagami::Object const &rhs) const {
      auto copy_lhs = lhs, copy_rhs = rhs;
      return kagami::management::type::CompareObjects(copy_lhs, copy_rhs);
    }
  };

  template <>
  struct not_equal_to<kagami::Object> {
    bool operator()(kagami::Object const &lhs, kagami::Object const &rhs) const {
      auto copy_lhs = lhs, copy_rhs = rhs;
      return !kagami::management::type::CompareObjects(copy_lhs, copy_rhs);
    }
  };
}

namespace kagami {
  using ObjectTable = unordered_map<Object, Object>;
  using ManagedTable = shared_ptr<ObjectTable>;
}

#define EXPORT_CONSTANT(ID) management::CreateConstantObject(#ID, Object(ID))


