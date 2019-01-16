#pragma once
#include "common.h"

namespace kagami {
  namespace util {
    bool IsOperatorToken(GenericTokenEnum token);
    int GetTokenPriority(GenericTokenEnum token);
    GenericTokenEnum GetGenericToken(string src);
    string GetRawString(string target);
    bool IsString(string target);
    bool IsGenericToken(string target);
    bool IsInteger(string target);
    bool IsFloat(string target);
    bool IsBlank(string target);
    bool IsSymbol(string target);
    bool IsBoolean(string target);
    TokenTypeEnum GetTokenType(string target);
    bool FindInStringGroup(string target, string source);
    vector<string> BuildStringVector(string source);
    char GetEscapeChar(char target);
    wchar_t GetEscapeCharW(wchar_t target);
    bool IsWideString(string target);
    string CombineStringVector(vector<string> target);
    void MakeBoolean(bool origin, string &target);
    bool IsDigit(char c);
    bool IsAlpha(char c);
  };
}