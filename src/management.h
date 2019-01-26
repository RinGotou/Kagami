#pragma once
#include "interface.h"

namespace kagami {
  namespace management {
    ContainerPool &GetContainerPool();
    ObjectContainer &GetCurrentContainer();
    ObjectContainer &CreateContainer();
    Object *FindObject(string id);
    Object *CreateObject(string id, Object &object);
    Object *CreateObject(string id, Object &&object);
    void CreateInterface(Interface temp);
    void CreateGenericInterface(Interface temp);
    bool DisposeManager();
    bool HasTailTokenRequest(GenericToken token);
    Interface Order(string id, string type = kTypeIdNull, int size = -1);
    Interface GetGenericInterface(GenericToken token);
    Object *CreateConstantObject(string id, Object &object);
    Object *CreateConstantObject(string id, Object &&object);

    namespace type {
      vector<string> GetMethods(string name);
      void NewType(string name, ObjectPolicy temp);
      shared_ptr<void> GetObjectCopy(Object &object);

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

        NewTypeSetup &InitMethods(std::initializer_list<Interface> &&rhs) {
          interfaces_ = rhs;
          string method_list("");

          for (auto &unit : interfaces_) {
            unit.SetDomain(type_name_);
            method_list.append(unit.GetId()).append("|");
          }

          if (method_list != "") {
            methods_ = method_list.substr(0, method_list.size() - 1);
          }

          return *this;
        }

        ~NewTypeSetup() {
          NewType(type_name_, ObjectPolicy(policy_, methods_));
          CreateInterface(constructor_);
          for (auto &unit : interfaces_) {
            CreateInterface(unit);
          }
        }
      };
    }
  }
}
