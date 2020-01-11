#pragma once
#include "common.h"

namespace kagami {
  namespace util {
    Terminator GetTerminatorCode(string src);
    bool IsBinaryOperator(Keyword token);
    bool IsMonoOperator(Keyword token);
    bool IsOperator(Keyword token);
    int GetTokenPriority(Keyword token);
    Keyword GetKeywordCode(string src);
    string GetRawString(string target);
    bool IsString(string target);
    bool IsIdentifier(string target);
    bool IsInteger(string target);
    bool IsFloat(string target);
    bool IsBlank(string target);
    bool IsSymbol(string target);
    bool IsBoolean(string target);
    StringType GetStringType(string target, bool ignore_symbol_rule = false);
    
    char GetEscapeChar(char target);
    wchar_t GetEscapeCharW(wchar_t target);
    bool IsWideString(string target);
    string CombineStringVector(vector<string> target);
    string MakeBoolean(bool origin);
    bool IsDigit(char c);
    bool IsAlpha(char c);
    bool IsPlainType(string type_id);
  };
}