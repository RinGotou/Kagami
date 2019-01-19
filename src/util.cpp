#include "util.h"

namespace kagami {
  namespace util {
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

    int GetTokenPriority(GenericTokenEnum token) {
      int result;
      switch (token) {
      case GT_BIND:
        result = 0;
        break;
      case GT_ADD:
      case GT_SUB:
        result = 2;
        break;
      case GT_MUL:
      case GT_DIV:
        result = 3;
        break;
      case GT_IS:
      case GT_LESS_OR_EQUAL:
      case GT_MORE_OR_EQUAL:
      case GT_NOT_EQUAL:
      case GT_MORE:
      case GT_LESS:
      case GT_AND:
      case GT_OR:
        result = 1;
        break;
      default:
        result = 4;
        break;
      }

      return result;
    }

    map<string, GenericTokenEnum> &GetGTBase() {
      using T = pair<string, GenericTokenEnum>;
      static map<string, GenericTokenEnum> base = {
        T(kStrIf          ,GT_IF),
        T(kStrNop         ,GT_NOP),
        T(kStrDef         ,GT_DEF),
        T(kStrEnd         ,GT_END),
        T(kStrBind        ,GT_BIND),
        T(kStrElse        ,GT_ELSE),
        T(kStrElif        ,GT_ELIF),
        T(kStrWhile       ,GT_WHILE),
        T(kStrAdd         ,GT_ADD),
        T(kStrSub         ,GT_SUB),
        T(kStrMul         ,GT_MUL),
        T(kStrDiv         ,GT_DIV),
        T(kStrIs          ,GT_IS),
        T(kStrAnd         ,GT_AND),
        T(kStrOr          ,GT_OR),
        T(kStrNot         ,GT_NOT),
        T(kStrBitAnd      ,GT_BIT_AND),
        T(kStrBitOr       ,GT_BIT_OR),
        T(kStrLessOrEqual ,GT_LESS_OR_EQUAL),
        T(kStrMoreOrEqual ,GT_MORE_OR_EQUAL),
        T(kStrNotEqual    ,GT_NOT_EQUAL),
        T(kStrMore        ,GT_MORE),
        T(kStrLess        ,GT_LESS),
        T(kStrReturn      ,GT_RETURN),
        T(kStrArray       ,GT_ARRAY),
        T(kStrTypeAssert  ,GT_ASSERT),
        T(kStrContinue    ,GT_CONTINUE),
        T(kStrBreak       ,GT_BREAK),
        T(kStrCase        ,GT_CASE),
        T(kStrWhen        ,GT_WHEN),
        T(kStrTypeAssertR ,GT_ASSERT_R),
        T(kStrTypeId      ,GT_TYPEID),
        T(kStrDir         ,GT_DIR)
      };
      return base;
    }

    GenericTokenEnum GetGenericToken(string src) {
      auto &base = GetGTBase();
      auto it = base.find(src);
      if (it != base.end()) return it->second;
      return GT_NUL;
    }

    bool IsString(string target) {
      if (target.empty()) return false;
      if (target.size() == 1) return false;
      return(target.front() == '\'' && target.back() == '\'');
    }

    bool IsGenericToken(string target) {
      if (target.empty()) return false;
      const auto head = target.front();

      if (!IsAlpha(head) && head != '_') return false;

      bool result = true;
      for (auto &unit : target) {
        if (!IsDigit(unit) &&
          !IsAlpha(unit) &&
          unit != '_') {
          result = false;
          break;
        }

      }
      return result;
    }

    bool IsInteger(string target) {
      if (target.empty()) return false;
      const auto head = target.front();

      if ((head == '-' || head == '+') &&
        target.size() == 1) {
        return false;
      }

      if (!IsDigit(head) &&
        head != '-' &&
        head != '+') {
        return false;
      }

      bool result = true;
      for (size_t i = 1; i < target.size(); ++i) {
        if (!IsDigit(target[i])) {
          result = false;
          break;
        }
      }
      return result;
    }

    bool IsFloat(string target) {
      if (target.empty()) return false;
      const auto head = target.front();
      bool dot = false;

      if ((head == '-' || head == '+') && target.size() == 1) return false;

      if (!IsDigit(head) &&
        head != '-' &&
        head != '+') {
        return false;
      }

      bool result = true;
      for (size_t i = 1; i < target.size(); ++i) {
        if (target[i] == '.' && dot == false) {
          dot = true;
        }
        else if (target[i] == '.' && dot == true) {
          result = false;
          break;
        }

        if (!IsDigit(target[i]) &&
          target[i] != '.') {
          result = false;
          break;
        }

        if (i == target.size() - 1 && !IsDigit(target[i])) {
          result = false;
          break;
        }
      }
      return result;
    }

    bool IsBlank(string target) {
      if (target.empty()) return false;
      bool result = true;
      for (auto &unit : target) {
        if (unit != ' ' && unit != '\t' && unit != '\r' && unit != '\n') {
          result = false;
          break;
        }
      }
      return result;
    }


    bool IsSymbol(string target) {
      static const regex kPatternSymbol(R"(==|<=|>=|!=|&&|\|\||[[:Punct:]])");
      if (target.empty()) return false;
      return std::regex_match(target, kPatternSymbol);
    }

    bool IsBoolean(string target) {
      return (target == "true" || target == "false");
    }

    TokenTypeEnum GetTokenType(string src) {
      TokenTypeEnum type = TokenTypeEnum::T_NUL;
      if (src.empty()) type = TokenTypeEnum::T_NUL;
      else if (IsBoolean(src)) type = TokenTypeEnum::T_BOOLEAN;
      else if (IsGenericToken(src)) type = TokenTypeEnum::T_GENERIC;
      else if (IsInteger(src)) type = TokenTypeEnum::T_INTEGER;
      else if (IsFloat(src)) type = TokenTypeEnum::T_FLOAT;
      else if (IsSymbol(src)) type = TokenTypeEnum::T_SYMBOL;
      else if (IsBlank(src)) type = TokenTypeEnum::T_BLANK;
      else if (IsString(src)) type = TokenTypeEnum::T_STRING;
      return type;
    }

    bool FindInStringGroup(string target, string source) {
      bool result = false;
      auto methods = BuildStringVector(source);
      for (auto &unit : methods) {
        if (unit == target) {
          result = true;
          break;
        }
      }
      return result;
    }

    vector<string> BuildStringVector(string source) {
      vector<string> result;
      string temp;
      for (auto unit : source) {
        if (unit == '|') {
          result.push_back(temp);
          temp.clear();
          continue;
        }
        temp.append(1, unit);
      }
      if (temp != "") result.push_back(temp);
      return result;
    }

    char GetEscapeChar(char target) {
      char result;
      switch (target) {
      case 't':result = '\t'; break;
      case 'n':result = '\n'; break;
      case 'r':result = '\r'; break;
      default:result = target; break;
      }
      return result;
    }

    wchar_t GetEscapeCharW(wchar_t target) {
      wchar_t result;
      switch (target) {
      case L't':result = L'\t'; break;
      case L'n':result = L'\n'; break;
      case L'r':result = L'\r'; break;
      default:result = target; break;
      }
      return result;
    }

    bool IsWideString(string target) {
      auto result = false;
      for (auto &unit : target) {
        if (unit < 0 || unit>127) {
          result = true;
          break;
        }
      }
      return result;
    }

    string CombineStringVector(vector<string> target) {
      string result;
      for (size_t i = 0; i < target.size(); ++i) {
        result = result + target[i] + "|";
      }
      result.pop_back();
      return result;
    }

    string GetRawString(string target) {
      string str = target.substr(1, target.size() - 2);
      string output;
      bool escape = false;
      for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '\\' && !escape) {
          escape = true;
          continue;
        }
        if (escape) {
          output.append(1, GetEscapeChar(str[i]));
          escape = false;
        }
        else {
          output.append(1, str[i]);
        }
      }
      return output;
    }

    void MakeBoolean(bool origin, string &target) {
      origin ?
        target = kStrTrue :
        target = kStrFalse;
    }

    bool IsDigit(char c) {
      return (c >= '0' && c <= '9');
    }

    bool IsAlpha(char c) {
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
  }
}