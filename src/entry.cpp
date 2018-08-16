#include "entry.h"

namespace kagami {
  bool Entry::Compare(Entry &target) const {
    return (target.id == this->id &&
      target.activity == this->activity &&
      target.parmMode == this->parmMode &&
      target.priority == this->priority &&
      this->specifictype == target.specifictype &&
      target.args == this->args);
  }

  Entry &Entry::SetSpecificType(string type) {
    this->specifictype = type;
    return *this;
  }

  Message Entry::Start(ObjectMap &map) const {
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
      if (src == kStrIf) return BG_IF;
      if (src == kStrNop) return BG_NOP;
      if (src == kStrDef) return BG_DEF;
      if (src == kStrRef) return BG_REF;
      if (src == kStrSub) return BG_SUB;
      if (src == kStrEnd) return BG_END;
      if (src == kStrVar) return BG_VAR;
      if (src == kStrSet) return BG_SET;
      if (src == kStrFor) return BG_FOR;
      if (src == kStrElse) return BG_ELSE;
      if (src == kStrElif) return BG_ELIF;
      if (src == kStrWhile) return BG_WHILE;
      if (src == kStrBinOp) return BG_BINOP;
      if (src == kStrCodeSub) return BG_CODE_SUB;
      if (src == kStrLeftSelfInc) return BG_LSELF_INC;
      if (src == kStrLeftSelfDec) return BG_LSELF_DEC;
      if (src == kStrRightSelfInc) return BG_RSELF_INC;
      if (src == kStrRightSelfDec) return BG_RSELF_DEC;
      if (src == kStrNull) return BG_NUL;
      return BG_NUL;
    }

    string GetGenTokenValue(GenericTokenEnum token) {
      string result;
      switch (token) {
      case BG_NOP:result = kStrNop; break;
      case BG_DEF:result = kStrDef; break;
      case BG_REF:result = kStrRef; break;
      case BG_CODE_SUB:result = kStrCodeSub; break;
      case BG_SUB:result = kStrSub; break;
      case BG_BINOP:result = kStrBinOp; break;
      case BG_IF:result = kStrIf; break;
      case BG_ELIF:result = kStrElif; break;
      case BG_END:result = kStrEnd; break;
      case BG_ELSE:result = kStrElse; break;
      case BG_VAR:result = kStrVar; break;
      case BG_SET:result = kStrSet; break;
      case BG_WHILE:result = kStrWhile; break;
      case BG_FOR:result = kStrFor; break;
      case BG_LSELF_INC:result = kStrLeftSelfInc; break;
      case BG_LSELF_DEC:result = kStrLeftSelfDec; break;
      case BG_RSELF_INC:result = kStrRightSelfInc; break;
      case BG_RSELF_DEC:result = kStrRightSelfDec; break;
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
      if (basicOpCode != BG_NUL) {
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
          && (size == -1 || size == int(unit.GetParameterSIze()))) {
          result = unit;
          break;
        }
      }
      return result;
    }

    size_t GetRequiredCount(string id) {
      OperatorCode opCode = GetOperatorCode(id);
      if (opCode == EQUAL) return Order(kStrSet, kTypeIdNull, -1).GetParameterSIze();
      if (opCode != NUL)   return Order(kStrBinOp, kTypeIdNull, -1).GetParameterSIze();
      auto provider = Order(id);
      if (provider.Good()) return provider.GetParameterSIze();

      return 0;
    }

    //#if defined(_WIN32)
    //    //from MSDN
    //    std::wstring s2ws(const std::string& s) {
    //      const auto slength = static_cast<int>(s.length()) + 1;
    //      const auto len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, nullptr, 0);
    //      auto *buf = new wchar_t[len];
    //      MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    //      std::wstring r(buf);
    //      delete[] buf;
    //      return r;
    //    }
    //#endif
  }
}