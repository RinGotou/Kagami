#include "management.h"

namespace kagami {
  namespace management {
    ///////////////////////////////////////////////////////////////
    //

    ///////////////////////////////////////////////////////////////
    //Inteface management
    map<GenericToken, Interface> &GetGenericInterfaceBase() {
      static map<GenericToken, Interface> base;
      return base;
    }

    map<string, InterfaceCollection> &GetInterfaceCollections() {
      static map<string, InterfaceCollection> collection_base;
      return collection_base;
    }

    void CreateNewInterface(Interface interface) {
      string domain = interface.GetTypeDomain();
      auto &collection_base = GetInterfaceCollections();
      auto it = collection_base.find(domain);

      if (it != collection_base.end()) {
        it->second.insert(std::make_pair(interface.GetId(), interface));
      }
      else {
        collection_base.insert(std::make_pair(domain, InterfaceCollection()));
        collection_base[domain].insert(std::make_pair(interface.GetId(), interface));
      }
    }

    Interface *FindInterface(string id, string domain) {
      auto &collection_base = GetInterfaceCollections();
      auto it = collection_base.find(domain);

      if (it != collection_base.end()) {
        auto dest_it = it->second.find(id);
        return dest_it != it->second.end() ?
          &dest_it->second : nullptr;
      }

      return nullptr;
    }

    void CreateGenericInterface(Interface temp) {
      GetGenericInterfaceBase().insert(pair<GenericToken, Interface>(
        temp.GetToken(), temp));
    }

    Interface *GetGenericInterface(GenericToken token) {
      auto &base = GetGenericInterfaceBase();
      map<GenericToken, Interface>::iterator it = base.find(token);
      if (it != base.end()) return &it->second;
      return nullptr;
    }
    ////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////
    //Constant object management

    ObjectContainer &GetConstantBase() {
      static ObjectContainer base;
      return base;
    }

    Object *CreateConstantObject(string id, Object &object) {
      ObjectContainer &base = GetConstantBase();

      if (base.Find(id) != nullptr) return nullptr;

      base.Add(id, object);
      auto result = base.Find(id);
      return result;
    }

    Object *CreateConstantObject(string id, Object &&object) {
      return CreateConstantObject(id, object);
    }

    Object GetConstantObject(string id) {
      ObjectContainer &base = GetConstantBase();
      auto ptr = base.Find(id);

      if (ptr != nullptr) {
        Object obj(type::GetObjectCopy(*ptr), ptr->GetTypeId());
        return obj;
      }

      return Object();
    }

    /////////////////////////////////////////////////////////////

    namespace type {
      map <string, ObjectPolicy> &GetPlannerBase() {
        static map<string, ObjectPolicy> base;
        return base;
      }

      vector<string> GetMethods(string id) {
        vector<string> result;
        const auto it = GetPlannerBase().find(id);

        if (it != GetPlannerBase().end()) {
          result = it->second.GetMethods();
        }
        return result;
      }

      bool CheckMethod(string func_id, string domain) {
        bool result = false;
        const auto it = GetPlannerBase().find(domain);

        if (it != GetPlannerBase().end()) {
          result = find_in_vector(func_id, it->second.GetMethods());
        }

        return result;
      }

      void NewType(string id, ObjectPolicy temp) {
        GetPlannerBase().insert(pair<string, ObjectPolicy>(id, temp));
      }

      shared_ptr<void> GetObjectCopy(Object &object) {
        //Ignore copying policy
        if (object.GetConstructorFlag() || object.IsMemberRef()) {
          return object.Get();
        }

        shared_ptr<void> result = nullptr;
        const auto option = object.GetTypeId();
        const auto it = GetPlannerBase().find(option);

        if (it != GetPlannerBase().end()) {
          result = it->second.CreateObjectCopy(object.Get());
        }
        return result;
      }

      bool CheckBehavior(Object obj, string method_str) {
        auto obj_methods = GetMethods(obj.GetTypeId());
        auto sample = BuildStringVector(method_str);
        bool result = true;
        for (auto &unit : sample) {
          if (!find_in_vector(unit, obj_methods)) {
            result = false;
            break;
          }
        }

        return result;
      }

      NewTypeSetup &NewTypeSetup::InitMethods(initializer_list<Interface> &&rhs) {
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

      NewTypeSetup::~NewTypeSetup() {
        NewType(type_name_, ObjectPolicy(policy_, methods_));
        CreateNewInterface(constructor_);
        for (auto &unit : interfaces_) {
          CreateNewInterface(unit);
        }
      }
    }
  }
}