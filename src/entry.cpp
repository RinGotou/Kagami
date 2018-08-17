#include "entry.h"

namespace kagami {
  bool Entry::Compare(Entry &target) const {
    return (target.id == this->id &&
      target.activity == this->activity &&
      target.parmMode == this->parmMode &&
      target.priority == this->priority &&
      this->type == target.type &&
      target.args == this->args);
  }

  Message Entry::Start(ObjectMap &map) const {
    if (placeholder) return Message();
    Message result;
    if (Good()) result = activity(map);
    else result.combo(kStrFatalError, kCodeIllegalCall, "Illegal entry.");
    return result;
  }

  namespace entry {
    list<ObjectManager> &GetObjectStack() {
      static list<ObjectManager> base;
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
      size_t count = GetObjectStack().size();
      list<ObjectManager> &base = GetObjectStack();

      while (!base.empty() && count > 0) {
        object = base[count - 1].Find(sign);
        if (object != nullptr) {
          break;
        }
        count--;
      }
      return object;
    }

    ObjectManager &GetCurrentManager() {
      return GetObjectStack().back();
    }

    Object *FindObjectInCurrentManager(string sign) {
      Object *object = nullptr;
      ObjectManager &base = GetObjectStack().back();

      while (!base.Empty()) {
        object = base.Find(sign);
        if (object != nullptr) {
          break;
        }
      }

      return object;
    }

    Object *CreateObject(string sign, Object &object) {
      ObjectManager &base = GetObjectStack().back();

      base.Add(sign, object);
      const auto result = base.Find(sign);
      return result;
    }

    string GetTypeId(const string sign) {
      auto result = kTypeIdNull;
      auto count = GetObjectStack().size();
      auto &base = GetObjectStack();

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
      while (!GetObjectStack().empty()) GetObjectStack().pop_back();
    }

    ObjectManager &CreateManager() {
      auto &base = GetObjectStack();
      base.push_back(std::move(ObjectManager()));
      return GetObjectStack().back();
    }

    bool DisposeManager() {
      if (!GetObjectStack().empty()) { GetObjectStack().pop_back(); }
      return GetObjectStack().empty();
    }

    GenericTokenEnum GetGenericToken(string src) {
      if (src == kStrIf) return GT_IF;
      if (src == kStrNop) return GT_NOP;
      if (src == kStrDef) return GT_DEF;
      if (src == kStrRef) return GT_REF;
      if (src == kStrSub) return GT_SUB;
      if (src == kStrEnd) return GT_END;
      if (src == kStrVar) return GT_VAR;
      if (src == kStrSet) return GT_SET;
      if (src == kStrBind) return GT_BIND;
      if (src == kStrFor) return GT_FOR;
      if (src == kStrElse) return GT_ELSE;
      if (src == kStrElif) return GT_ELIF;
      if (src == kStrWhile) return GT_WHILE;
      if (src == kStrBinOp) return GT_BINOP;
      if (src == kStrCodeSub) return GT_CODE_SUB;
      if (src == kStrLeftSelfInc) return GT_LSELF_INC;
      if (src == kStrLeftSelfDec) return GT_LSELF_DEC;
      if (src == kStrRightSelfInc) return GT_RSELF_INC;
      if (src == kStrRightSelfDec) return GT_RSELF_DEC;
      if (src == kStrNop) return GT_NOP;
      if (src == kStrNull) return GT_NUL;
      return GT_NUL;
    }

    string GetGenTokenValue(GenericTokenEnum token) {
      string result;
      switch (token) {
      case GT_NOP:result = kStrNop; break;
      case GT_DEF:result = kStrDef; break;
      case GT_REF:result = kStrRef; break;
      case GT_CODE_SUB:result = kStrCodeSub; break;
      case GT_SUB:result = kStrSub; break;
      case GT_BINOP:result = kStrBinOp; break;
      case GT_IF:result = kStrIf; break;
      case GT_ELIF:result = kStrElif; break;
      case GT_END:result = kStrEnd; break;
      case GT_ELSE:result = kStrElse; break;
      case GT_VAR:result = kStrVar; break;
      case GT_SET:result = kStrSet; break;
      case GT_BIND:result = kStrBind; break;
      case GT_WHILE:result = kStrWhile; break;
      case GT_FOR:result = kStrFor; break;
      case GT_LSELF_INC:result = kStrLeftSelfInc; break;
      case GT_LSELF_DEC:result = kStrLeftSelfDec; break;
      case GT_RSELF_INC:result = kStrRightSelfInc; break;
      case GT_RSELF_DEC:result = kStrRightSelfDec; break;
      case GT_NUL:result = kStrNull; break;
      default:break;
      }
      return result;
    }

    OperatorCode GetOperatorCode(string src) {
      if (src == "+")  return ADD;
      if (src == "-")  return SUB;
      if (src == "*")  return MUL;
      if (src == "/")  return DIV;
      if (src == "=")  return EQUAL;
      if (src == "==") return IS;
      if (src == "<=") return LESS_OR_EQUAL;
      if (src == ">=") return MORE_OR_EQUAL;
      if (src == "!=") return NOT_EQUAL;
      if (src == ">")  return MORE;
      if (src == "<")  return LESS;
      if (src == "++") return SELFINC;
      if (src == "--") return SELFDEC;
      return NUL;
    }

    void Inject(Entry temp) {
      GetEntryBase().emplace_back(temp);
    }

    void LoadGenProvider(GenericTokenEnum token, Entry temp) {
      GetGenProviderBase().insert(pair<GenericTokenEnum, Entry>(
        token, temp));
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
      OperatorCode opCode = GetOperatorCode(id);

      if (opCode == EQUAL)    return Order(kStrSet);
      else if (opCode != NUL) return Order(kStrBinOp);

      Entry result;
      //TODO:rewrite here
      for (auto &unit : base) {
        if (id == unit.GetId() && type == unit.GetSpecificType()
          && (size == -1 || size == int(unit.GetParmSize()))) {
          result = unit;
          break;
        }
      }
      return result;
    }

    size_t GetRequiredCount(string id) {
      OperatorCode opCode = GetOperatorCode(id);
      Entry ent;
      size_t count;
      switch (opCode) {
      case SELFINC:
      case SELFDEC:
        count = 1;
        break;
      case NUL:
        ent = Order(id);
        if (ent.Good()) count = ent.GetParmSize();
        else count = 0;
        break;
      default:
        count = 2;
        break;
      }
      return count;
    }
  }
}