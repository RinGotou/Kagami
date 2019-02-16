#pragma once
#include "common.h"

namespace kagami {
  namespace util {
    bool IsBinaryOperator(GenericToken token);
    bool IsMonoOperator(GenericToken token);
    bool IsOperator(GenericToken token);
    int GetTokenPriority(GenericToken token);
    GenericToken GetGenericToken(string src);
    string GetRawString(string target);
    bool IsString(string target);
    bool IsGenericToken(string target);
    bool IsInteger(string target);
    bool IsFloat(string target);
    bool IsBlank(string target);
    bool IsSymbol(string target);
    bool IsBoolean(string target);
    TokenType GetTokenType(string target, bool ignore_symbol_rule = false);
    
    char GetEscapeChar(char target);
    wchar_t GetEscapeCharW(wchar_t target);
    bool IsWideString(string target);
    string CombineStringVector(vector<string> target);
    string MakeBoolean(bool origin);
    bool IsDigit(char c);
    bool IsAlpha(char c);
  };
}