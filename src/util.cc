#include "util.h"

namespace kagami {
  namespace util {
    bool IsOperatorToken(GenericToken token) {
      bool result;
      switch (token) {
      case kTokenPlus:
      case kTokenMinus:
      case kTokenTimes:
      case kTokenDivide:
      case kTokenEquals:
      case kTokenLessOrEqual:
      case kTokenGreaterOrEqual:
      case kTokenNotEqual:
      case kTokenGreater:
      case kTokenLess:
      case kTokenAnd:
      case kTokenOr:
      case kTokenNot:
      case kTokenBitAnd:
      case kTokenBitOr:
        result = true;
        break;
      default:
        result = false;
        break;
      }
      return result;
    }

    bool IsMonoOperator(GenericToken token) {
      bool result;
      switch (token) {
      case kTokenNot:
      case kTokenBitNot:
        result = true;
        break;
      default:
        result = false;
        break;
      }

      return result;
    }

    int GetTokenPriority(GenericToken token) {
      int result;
      switch (token) {
      case kTokenBind:
        result = 0;
        break;
      case kTokenPlus:
      case kTokenMinus:
        result = 2;
        break;
      case kTokenTimes:
      case kTokenDivide:
        result = 3;
        break;
      case kTokenEquals:
      case kTokenLessOrEqual:
      case kTokenGreaterOrEqual:
      case kTokenNotEqual:
      case kTokenGreater:
      case kTokenLess:
      case kTokenAnd:
      case kTokenOr:
        result = 1;
        break;
      default:
        result = 4;
        break;
      }

      return result;
    }

    map<string, GenericToken> &GetGTBase() {
      using T = pair<string, GenericToken>;
      static map<string, GenericToken> base = {
        T(kStrIf             ,kTokenIf),
        T(kStrFn             ,kTokenFn),
        T(kStrEnd            ,kTokenEnd),
        T(kStrElse           ,kTokenElse),
        T(kStrElif           ,kTokenElif),
        T(kStrWhile          ,kTokenWhile),
        T(kStrPlus           ,kTokenPlus),
        T(kStrMinus          ,kTokenMinus),
        T(kStrTimes          ,kTokenTimes),
        T(kStrDiv            ,kTokenDivide),
        T(kStrIs             ,kTokenEquals),
        T(kStrAnd            ,kTokenAnd),
        T(kStrOr             ,kTokenOr),
        T(kStrNot            ,kTokenNot),
        T(kStrBitAnd         ,kTokenBitAnd),
        T(kStrBitOr          ,kTokenBitOr),
        T(kStrBitNot         ,kTokenBitNot),
        T(kStrLessOrEqual    ,kTokenLessOrEqual),
        T(kStrGreaterOrEqual ,kTokenGreaterOrEqual),
        T(kStrNotEqual       ,kTokenNotEqual),
        T(kStrGreater        ,kTokenGreater),
        T(kStrLess           ,kTokenLess),
        T(kStrReturn         ,kTokenReturn),
        T(kStrArray          ,kTokenInitialArray),
        T(kStrTypeAssert     ,kTokenAssert),
        T(kStrContinue       ,kTokenContinue),
        T(kStrBreak          ,kTokenBreak),
        T(kStrCase           ,kTokenCase),
        T(kStrWhen           ,kTokenWhen),
        T(kStrTypeAssertR    ,kTokenAssertR),
        T(kStrTypeId         ,kTokenTypeId),
        T(kStrDir            ,kTokenDir)
      };
      return base;
    }

    GenericToken GetGenericToken(string src) {
      auto &base = GetGTBase();
      auto it = base.find(src);
      if (it != base.end()) return it->second;
      return kTokenNull;
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
        if (!IsDigit(unit) && !IsAlpha(unit) && unit != '_') {
          result = false;
          break;
        }

      }
      return result;
    }

    bool IsInteger(string target) {
      if (target.empty()) return false;
      const auto head = target.front();

      if (compare(head, { '-','+' }) && target.size() == 1) {
        return false;
      }

      if (!IsDigit(head) && !compare(head, {'-', '+'})) {
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

      if (compare(head, { '-','+' }) && target.size() == 1) {
        return false;
      }

      if (!IsDigit(head) && !compare(head, { '-', '+' })) {
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
        if (!compare(unit, { ' ','\t','\r','\n' })) {
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
      return compare(target, { "true","false" });
    }

    TokenType GetTokenType(string src, bool ignore_symbol_rule) {
      TokenType type = kTokenTypeNull;
      if (src.empty())              type = kTokenTypeNull;
      else if (IsBoolean(src))      type = kTokenTypeBool;
      else if (IsGenericToken(src)) type = kTokenTypeGeneric;
      else if (IsInteger(src))      type = kTokenTypeInt;
      else if (IsFloat(src))        type = kTokenTypeFloat;
      else if (IsBlank(src))        type = kTokenTypeBlank;
      else if (IsString(src))       type = kTokenTypeString;

      //holy xxxx -> std::regex
      if (!ignore_symbol_rule) {
        if (IsSymbol(src)) type = kTokenTypeSymbol;
      }
      return type;
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
        if (unit < 0 || unit > 127) {
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

    string MakeBoolean(bool origin) {
      return origin ? kStrTrue : kStrFalse;
    }

    bool IsDigit(char c) {
      return (c >= '0' && c <= '9');
    }

    bool IsAlpha(char c) {
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
  }
}