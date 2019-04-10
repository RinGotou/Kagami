#pragma once
#include "interface.h"

namespace kagami {
  namespace management {
    using InterfaceCollection = map<string, Interface>;

    //long GetStackDepthLimit();

    void CreateNewInterface(Interface interface);
    Interface *FindInterface(string id, string domain = kTypeIdNull);
    void CreateGenericInterface(Interface temp);
    Interface *GetGenericInterface(GenericToken token);

    Object *CreateConstantObject(string id, Object &object);
    Object *CreateConstantObject(string id, Object &&object);
    Object GetConstantObject(string id);

    namespace type {
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

      class ObjectPolicy {
      private:
        CopyingPolicy copying_policy_;
        shared_ptr<HasherInterface> hasher_;
        vector<string> methods_;

      public:
        ObjectPolicy() = delete;

        ObjectPolicy(CopyingPolicy copying_policy, string methods, 
          shared_ptr<HasherInterface> hasher = nullptr) :
          copying_policy_(copying_policy),
          methods_(BuildStringVector(methods)),
          hasher_(std::move(hasher)) {}

        shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) const;

        vector<string> &GetMethods() {
          return methods_;
        }

        shared_ptr<HasherInterface> &GetHasher() {
          return hasher_;
        }
      };

      vector<string> GetMethods(string id);
      bool CheckMethod(string func_id, string domain);
      size_t GetHash(Object &obj);
      bool IsHashable(Object &obj);
      void NewType(string id, ObjectPolicy temp);
      shared_ptr<void> GetObjectCopy(Object &object);
      bool CheckBehavior(Object obj, string method_str);

      class NewTypeSetup {
      private:
        string type_name_;
        string methods_;
        CopyingPolicy policy_;
        shared_ptr<HasherInterface> hasher_;
        vector<Interface> interfaces_;
        Interface constructor_;

      public:
        NewTypeSetup() = delete;

        template <class HasherType>
        NewTypeSetup(string type_name, CopyingPolicy policy,
          HasherType hasher) :
          type_name_(type_name), policy_(policy),
          hasher_(new HasherType(hasher)) {
          static_assert(is_base_of<HasherInterface, HasherType>::value,
            "Wrong hasher type.");
        }

        NewTypeSetup(string type_name, CopyingPolicy policy) :
          type_name_(type_name), policy_(policy), hasher_(nullptr) {}

        NewTypeSetup &InitConstructor(Interface interface) {
          constructor_ = interface;
          return *this;
        }

        NewTypeSetup &InitMethods(initializer_list<Interface> &&rhs);
        ~NewTypeSetup();
      };
    }
  }
}

#define EXPORT_CONSTANT(ID) management::CreateConstantObject(#ID, Object(ID))


