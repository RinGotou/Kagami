#include "kit.h"

namespace kagami {
  bool Kit::IsString(string target) {
    if (target.empty()) return false;
    if (target.size() == 1) return false;
    return(target.front() == '\'' && target.back() == '\'');
  }

  bool Kit::IsGenericToken(string target) {
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

  bool Kit::IsInteger(string target) {
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

  bool Kit::IsFloat(string target) {
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

  bool Kit::IsBlank(string target) {
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


  bool Kit::IsSymbol(string target) {
    static const regex kPatternSymbol(R"(\+\+|--|==|<=|>=|!=|&&|\|\||[[:Punct:]])");
    if (target.empty()) return false;
    return std::regex_match(target, kPatternSymbol);
  }

  bool Kit::IsBoolean(string target) {
    return (target == "true" || target == "false");
  }

  TokenTypeEnum Kit::GetTokenType(string src) {
    TokenTypeEnum type = TokenTypeEnum::T_NUL;
    if (src == kStrNull || src.empty()) type = TokenTypeEnum::T_NUL;
    else if (IsBoolean(src)) type = TokenTypeEnum::T_BOOLEAN;
    else if (IsGenericToken(src)) type = TokenTypeEnum::T_GENERIC;
    else if (IsInteger(src)) type = TokenTypeEnum::T_INTEGER;
    else if (IsFloat(src)) type = TokenTypeEnum::T_FLOAT;
    else if (IsSymbol(src)) type = TokenTypeEnum::T_SYMBOL;
    else if (IsBlank(src)) type = TokenTypeEnum::T_BLANK;
    else if (IsString(src)) type = TokenTypeEnum::T_STRING;
    return type;
  }

  bool Kit::FindInStringGroup(string target, string source) {
    bool result = false;
    auto methods = Kit::BuildStringVector(source);
    for (auto &unit : methods) {
      if (unit == target) {
        result = true;
        break;
      }
    }
    return result;
  }

  vector<string> Kit::BuildStringVector(string source) {
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
    if (temp != kStrEmpty) result.push_back(temp);
    return result;
  }

  char Kit::GetEscapeChar(char target) {
    char result;
    switch (target) {
    case 't':result = '\t'; break;
    case 'n':result = '\n'; break;
    case 'r':result = '\r'; break;
    default:result = target; break;
    }
    return result;
  }

  wchar_t Kit::GetEscapeCharW(wchar_t target) {
    wchar_t result;
    switch (target) {
    case L't':result = L'\t'; break;
    case L'n':result = L'\n'; break;
    case L'r':result = L'\r'; break;
    default:result = target; break;
    }
    return result;
  }

  bool Kit::IsWideString(string target) {
    auto result = false;
    for (auto &unit : target) {
      if (unit < 0 || unit>127) {
        result = true;
        break;
      }
    }
    return result;
  }

  string Kit::CombineStringVector(vector<string> target) {
    string result;
    for (size_t i = 0; i < target.size(); ++i) {
      result = result + target[i] + "|";
    }
    result.pop_back();
    return result;
  }

  string Kit::GetRawString(string target) {
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

  void Kit::MakeBoolean(bool origin, string &target) {
    origin ?
      target = kStrTrue :
      target = kStrFalse;
  }
}