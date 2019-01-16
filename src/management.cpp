#pragma once
#include "management.h"

namespace kagami {
  namespace management {
    list<ObjectContainer> &GetContainerPool() {
      static list<ObjectContainer> base;
      return base;
    }

    vector<Entry> &GetEntryBase() {
      static vector<Entry> base;
      return base;
    }

    map<GenericTokenEnum, Entry> &GetGenProviderBase() {
      static map<GenericTokenEnum, Entry> base;
      return base;
    }

    Object *FindObject(string id) {
      Object *object = nullptr;
      size_t count = GetContainerPool().size();
      list<ObjectContainer> &base = GetContainerPool();

      while (!base.empty() && count > 0) {
        object = base[count - 1].Find(id);
        if (object != nullptr) {
          break;
        }
        count--;
      }

      return object;
    }

    ObjectContainer &GetCurrentContainer() {
      return GetContainerPool().back();
    }

    Object *CreateObject(string id, Object &object) {
      ObjectContainer &base = GetContainerPool().back();
      if (base.Find(id) != nullptr) {
        return nullptr;
      }
      base.Add(id, object);
      const auto result = base.Find(id);
      return result;
    }

    ObjectContainer &CreateContainer() {
      auto &base = GetContainerPool();
      base.push_back(std::move(ObjectContainer()));
      return GetContainerPool().back();
    }

    bool DisposeManager() {
      auto &base = GetContainerPool();
      if (!base.empty()) { base.pop_back(); }
      return base.empty();
    }



    bool HasTailTokenRequest(GenericTokenEnum token) {
      return (token == GT_IF || token == GT_WHILE || token == GT_CASE);
    }

    void AddEntry(Entry temp) {
      GetEntryBase().emplace_back(temp);
    }

    void AddGenericEntry(Entry temp) {
      GetGenProviderBase().insert(pair<GenericTokenEnum, Entry>(
        temp.GetTokenEnum(), temp));
    }

    Entry GetGenericProvider(GenericTokenEnum token) {
      auto &base = GetGenProviderBase();
      map<GenericTokenEnum, Entry>::iterator it = base.find(token);
      if (it != base.end()) return it->second;
      return Entry();
    }


    Entry Order(string id, string type, int size) {
      GenericTokenEnum basicOpCode = util::GetGenericToken(id);
      if (basicOpCode != GT_NUL) {
        return GetGenericProvider(basicOpCode);
      }

      vector<Entry> &base = GetEntryBase();
      Entry result;
      bool ignore_type = (type == kTypeIdNull);
      //TODO:rewrite here
      for (auto &unit : base) {
        bool typeChecking = (ignore_type || type == unit.GetTypeDomain());
        bool sizeChecking = (size == -1 || size == int(unit.GetParmSize()));
        if (id == unit.GetId() && typeChecking && sizeChecking) {
          result = unit;
          break;
        }
      }
      return result;
    }

    namespace type {
      map <string, ObjectCopyingPolicy> &GetPlannerBase() {
        static map<string, ObjectCopyingPolicy> base;
        return base;
      }

      shared_ptr<void> GetObjectCopy(Object &object) {
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

      string GetMethods(string name) {
        string result;
        const auto it = GetPlannerBase().find(name);

        if (it != GetPlannerBase().end()) {
          result = it->second.GetMethods();
        }
        return result;
      }

      void AddTemplate(string name, ObjectCopyingPolicy temp) {
        GetPlannerBase().insert(pair<string, ObjectCopyingPolicy>(name, temp));
      }
    }
  }
}