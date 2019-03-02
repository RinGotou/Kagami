#include "management.h"

namespace kagami {
  namespace management {
    ObjectContainer &GetConstantBase() {
      static ObjectContainer base;
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

    Interface FindInterface(string id, string domain) {
      auto &collection_base = GetInterfaceCollections();
      auto it = collection_base.find(domain);

      if (it != collection_base.end()) {
        auto dest_it = it->second.find(id);
        return Interface(dest_it != it->second.end() ?
          dest_it->second : Interface());
      }

      return Interface();
    }

    vector<Interface> &GetInterfaceBase() {
      static vector<Interface> base;
      return base;
    }

    map<GenericToken, Interface> &GetGenericInterfaceBase() {
      static map<GenericToken, Interface> base;
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

    bool NeedEndToken(GenericToken token) {
      return (token == kTokenIf || token == kTokenWhile || token == kTokenCase);
    }

    void CreateGenericInterface(Interface temp) {
      GetGenericInterfaceBase().insert(pair<GenericToken, Interface>(
        temp.GetToken(), temp));
    }

    Interface GetGenericInterface(GenericToken token) {
      auto &base = GetGenericInterfaceBase();
      map<GenericToken, Interface>::iterator it = base.find(token);
      if (it != base.end()) return it->second;
      return Interface();
    }

    namespace type {
      map <string, ObjectPolicy> &GetPlannerBase() {
        static map<string, ObjectPolicy> base;
        return base;
      }

      shared_ptr<void> GetObjectCopy(Object &object) {
        //Ignore copying policy
        if (object.GetConstructorFlag()) {
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

      vector<string> GetMethods(string name) {
        vector<string> result;
        const auto it = GetPlannerBase().find(name);

        if (it != GetPlannerBase().end()) {
          result = it->second.GetMethods();
        }
        return result;
      }

      void NewType(string name, ObjectPolicy temp) {
        GetPlannerBase().insert(pair<string, ObjectPolicy>(name, temp));
      }
    }
  }
}