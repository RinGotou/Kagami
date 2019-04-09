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

      vector<string> GetMethods(string name);
      void NewType(string name, ObjectPolicy temp);
      shared_ptr<void> GetObjectCopy(Object &object);
      bool CheckBehavior(Object obj, string method_str);

      class NewTypeSetup {
      private:
        string type_name_;
        string methods_;
        CopyingPolicy policy_;
        vector<Interface> interfaces_;
        Interface constructor_;

      public:
        NewTypeSetup() = delete;

        NewTypeSetup(string type_name, CopyingPolicy policy) :
          type_name_(type_name), policy_(policy) {}

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

#define SUB_TYPE(ID) EXPORT_CONSTANT(ID); NewTypeSetup(ID,
#define SET_MEM_POLICY(POLICY) POLICY)
#define SET_CONSTRUCTOR(CONS) .InitConstructor(CONS)
#define BLOCK_METHOD .InitMethods({
#define END_METHOD })
#define END_SUB ;
