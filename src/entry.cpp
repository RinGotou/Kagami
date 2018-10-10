#include "entry.h"
#include <iostream>

namespace kagami {
  bool Entry::Compare(Entry &target) const {
    return (target.id == this->id &&
      target.activity == this->activity &&
      target.parmMode == this->parmMode &&
      target.priority == this->priority &&
      this->type == target.type &&
      target.args == this->args);
  }

  Message Entry::Start(ObjectMap &objMap) const {
    if (placeholder) return Message();
    Message result;
    if (userFunc) {
      objMap[kStrUserFunc] = Object()
        .Manage(id)
        .SetMethods(type::GetMethods(kTypeIdRawString))
        .SetTokenType(T_GENERIC);
    }
    if (Good()) {
      result = activity(objMap);
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, "Illegal entry.");
    }
    return result;
  }

  namespace entry {
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

    Object *FindObject(string sign) {
      Object *object = nullptr;
      size_t count = GetContainerPool().size();
      list<ObjectContainer> &base = GetContainerPool();

      while (!base.empty() && count > 0) {
        object = base[count - 1].Find(sign);
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

    Object *FindObjectInCurrentContainer(string sign) {
      Object *object = nullptr;
      ObjectContainer &base = GetContainerPool().back();

      while (!base.Empty()) {
        object = base.Find(sign);
        if (object != nullptr) {
          break;
        }
      }

      return object;
    }

    Object *CreateObject(string sign, Object &object) {
      ObjectContainer &base = GetContainerPool().back();
      if (base.Find(sign) != nullptr) {
        return nullptr;
      }
      base.Add(sign, object);
      const auto result = base.Find(sign);
      return result;
    }

    string GetTypeId(const string sign) {
      auto result = kTypeIdNull;
      auto count = GetContainerPool().size();
      auto &base = GetContainerPool();

      while (count > 0) {
        const auto object = base[count - 1].Find(sign);
        if (object != nullptr) {
          result = object->GetTypeId();
        }
        count--;
      }

      return result;
    }

    void ResetObject() {
      while (!GetContainerPool().empty()) GetContainerPool().pop_back();
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

    map<string, GenericTokenEnum> &GetGTBase() {
      using T = pair<string, GenericTokenEnum>;
      static map<string, GenericTokenEnum> base = {
        T(kStrIf,GT_IF),
        T(kStrNop,GT_NOP),
        T(kStrDef,GT_DEF),
        T(kStrRef,GT_REF),
        T(kStrEnd,GT_END),
        T(kStrSet,GT_SET),
        T(kStrBind,GT_BIND),
        T(kStrFor,GT_FOR),
        T(kStrElse,GT_ELSE),
        T(kStrElif,GT_ELIF),
        T(kStrWhile,GT_WHILE),
        T(kStrCodeSub,GT_CODE_SUB),
        T(kStrLeftSelfInc,GT_LSELF_INC),
        T(kStrLeftSelfDec,GT_LSELF_DEC),
        T(kStrRightSelfInc,GT_RSELF_INC),
        T(kStrRightSelfDec,GT_RSELF_DEC),
        T(kStrAdd,GT_ADD),
        T(kStrSub,GT_SUB),
        T(kStrMul,GT_MUL),
        T(kStrDiv,GT_DIV),
        T(kStrIs,GT_IS),
        T(kStrAnd,GT_AND),
        T(kStrOr,GT_OR),
        T(kStrNot,GT_NOT),
        T(kStrBitAnd,GT_BIT_AND),
        T(kStrBitOr,GT_BIT_OR),
        T(kStrLessOrEqual,GT_LESS_OR_EQUAL),
        T(kStrMoreOrEqual,GT_MORE_OR_EQUAL),
        T(kStrNotEqual,GT_NOT_EQUAL),
        T(kStrMore,GT_MORE),
        T(kStrLess,GT_LESS),
        T(kStrReturn,GT_RETURN),
        T(kStrArray,GT_ARRAY),
        T(kStrTypeAssert,GT_TYPE_ASSERT),
        T(kStrContinue,GT_CONTINUE),
        T(kStrBreak,GT_BREAK),
        T(kStrCase,GT_CASE),
        T(kStrWhen,GT_WHEN)
      };
      return base;
    }

    GenericTokenEnum GetGenericToken(string src) {
      auto &base = GetGTBase();
      auto it = base.find(src);
      if (it != base.end()) return it->second;
      return GT_NUL;
    }

    bool IsOperatorToken(GenericTokenEnum token) {
      bool result;
      switch (token) {
      case GT_ADD:
      case GT_SUB:
      case GT_MUL:
      case GT_DIV:
      case GT_IS:
      case GT_LESS_OR_EQUAL:
      case GT_MORE_OR_EQUAL:
      case GT_NOT_EQUAL:
      case GT_MORE:
      case GT_LESS:
      case GT_AND:
      case GT_OR:
      case GT_NOT:
      case GT_BIT_AND:
      case GT_BIT_OR:
        result = true;
        break;
      default:
        result = false;
        break;
      }
      return result;
    }

    string GetGenTokenValue(GenericTokenEnum token) {
      auto &base = GetGTBase();
      string result = kStrNull;
      for (auto &unit : base) {
        if (unit.second == token) {
          result = unit.first;
          break;
        }
      }
      return result;
    }

    bool HasTailTokenRequest(GenericTokenEnum token) {
      return (token == GT_IF || token == GT_WHILE || token == GT_CASE);
    }

    map<string, OperatorCode> &GetOPBase() {
      using T = pair<string, OperatorCode>;
      static map<string, OperatorCode> base = {
        T("+",ADD),
        T("-",SUB),
        T("*",MUL),
        T("/",DIV),
        T("=",EQUAL),
        T("==",IS),
        T("<=",LESS_OR_EQUAL),
        T(">=",MORE_OR_EQUAL),
        T("!=",NOT_EQUAL),
        T(">",MORE),
        T("<",LESS),
        T("++",SELFINC),
        T("--",SELFDEC),
        T("&&",AND),
        T("||",OR),
        T("!",NOT),
        T("&",BIT_AND),
        T("|",BIT_OR)
      };
      return base;
    }

    OperatorCode GetOperatorCode(string src) {
      auto &base = GetOPBase();
      auto it = base.find(src);
      if (it != base.end()) return base[src];
      return NUL;
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
      GenericTokenEnum basicOpCode = GetGenericToken(id);
      if (basicOpCode != GT_NUL) {
        return GetGenericProvider(basicOpCode);
      }

      vector<Entry> &base = GetEntryBase();
      Entry result;
      bool ignoreType = (type == kTypeIdNull);
      //TODO:rewrite here
      for (auto &unit : base) {
        bool typeChecking = (ignoreType || type == unit.GetSpecificType());
        bool sizeChecking = (size == -1 || size == int(unit.GetParmSize()));
        if (id == unit.GetId() && typeChecking && sizeChecking) {
          result = unit;
          break;
        }
      }
      return result;
    }
  }

  namespace type {
    map <string, ObjectPlanner> &GetPlannerBase() {
      static map<string, ObjectPlanner> base;
      return base;
    }

    shared_ptr<void> GetObjectCopy(Object &object) {
      if (object.ConstructorFlag()) {
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

    ObjectPlanner *GetPlanner(const string name) {
      ObjectPlanner *result = nullptr;
      const auto it = GetPlannerBase().find(name);

      if (it != GetPlannerBase().end()) {
        result = &(it->second);
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

    void AddTemplate(string name, ObjectPlanner temp) {
      GetPlannerBase().insert(pair<string, ObjectPlanner>(name, temp));
    }

    void DisposeTemplate(const string name) {
      const auto it = GetPlannerBase().find(name);
      if (it != GetPlannerBase().end()) GetPlannerBase().erase(it);
    }
  }
}