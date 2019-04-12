#include "management.h"

namespace kagami {
  namespace management {
    ///////////////////////////////////////////////////////////////
    //

    ///////////////////////////////////////////////////////////////
    //Inteface management

    map<string, InterfaceCollection> &GetInterfaceCollections() {
      static map<string, InterfaceCollection> collection_base;
      return collection_base;
    }

    unordered_map<string, InterfaceHashMap> &GetInterfaceCache() {
      static unordered_map<string, InterfaceHashMap> cache;
      return cache;
    }

    void BuildInterfaceCache(string domain) {
      auto &base = GetInterfaceCache();
      auto &col = GetInterfaceCollections().at(domain);

      for (auto &unit : col) {
        base[domain].insert(make_pair(unit.first, &unit.second));
      }
    }

    void CreateNewInterface(Interface interface) {
      string domain = interface.GetTypeDomain();
      auto &collection_base = GetInterfaceCollections();
      auto it = collection_base.find(domain);

      if (it != collection_base.end()) {
        it->second.insert(std::make_pair(interface.GetId(), interface));
      }
      else {
        collection_base.insert(make_pair(domain, InterfaceCollection()));
        collection_base[domain].insert(std::make_pair(interface.GetId(), interface));
      }

      BuildInterfaceCache(domain);
    }

    Interface *FindInterface(string id, string domain) {
      auto &cache = GetInterfaceCache();
      auto it = cache.find(domain);

      if (it != cache.end()) {
        auto dest = it->second.find(id);
        if (dest != it->second.end()) {
          return dest->second;
        }
      }

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
        Object obj = type::CreateObjectCopy(*ptr);
        return obj;
      }

      return Object();
    }

    /////////////////////////////////////////////////////////////

    namespace type {
      shared_ptr<void> ObjectPolicy::CreateObjectCopy(shared_ptr<void> target) const {
        shared_ptr<void> result = nullptr;
        if (target != nullptr) {
          result = copying_policy_(target);
        }
        return result;
      }

      map <string, ObjectPolicy> &GetObjPolicyCollection() {
        static map<string, ObjectPolicy> base;
        return base;
      }

      vector<string> GetMethods(string id) {
        vector<string> result;
        const auto it = GetObjPolicyCollection().find(id);

        if (it != GetObjPolicyCollection().end()) {
          result = it->second.GetMethods();
        }
        return result;
      }

      bool CheckMethod(string func_id, string domain) {
        bool result = false;
        const auto it = GetObjPolicyCollection().find(domain);

        if (it != GetObjPolicyCollection().end()) {
          result = find_in_vector(func_id, it->second.GetMethods());
        }

        return result;
      }

      size_t GetHash(Object &obj) {
        auto &base = GetObjPolicyCollection();
        const auto it = base.find(obj.GetTypeId());
        auto hasher = it->second.GetHasher();
        return hasher->Get(obj.Deref().Get());
      }

      bool IsHashable(Object &obj) {
        bool result = false;
        auto &base = GetObjPolicyCollection();
        const auto it = base.find(obj.GetTypeId());

        if (it != base.end()) {
          result = (it->second.GetHasher() != nullptr);
        }

        return result;
      }

      void NewType(string id, ObjectPolicy temp) {
        GetObjPolicyCollection().insert(pair<string, ObjectPolicy>(id, temp));
      }

      Object CreateObjectCopy(Object &object) {
        if (object.GetConstructorFlag() || object.IsMemberRef()) {
          return object;
        }

        Object result;
        const auto it = GetObjPolicyCollection().find(object.GetTypeId());
        if (it != GetObjPolicyCollection().end()) {
          result.Manage(it->second.CreateObjectCopy(object.Get()),
            object.GetTypeId());
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
        NewType(type_name_, ObjectPolicy(policy_, methods_, hasher_));
        CreateNewInterface(constructor_);
        for (auto &unit : interfaces_) {
          CreateNewInterface(unit);
        }
      }
    }
  }
}