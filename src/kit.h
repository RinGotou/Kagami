#pragma once
#include "common.h"

namespace kagami {
  /*Kit Class
  this class contains many useful template or tiny function, and
  create script processing workspace.
  */
  class Kit {
  public:
    static string GetRawString(string target);
    static bool IsString(string target);
    static bool IsGenericToken(string target);
    static bool IsInteger(string target);
    static bool IsFloat(string target);
    static bool IsBlank(string target);
    static bool IsSymbol(string target);
    static bool IsBoolean(string target);
    static TokenTypeEnum GetTokenType(string target);
    static bool FindInStringGroup(string target, string source);
    static vector<string> BuildStringVector(string source);
    static char GetEscapeChar(char target);
    static wchar_t GetEscapeCharW(wchar_t target);
    static bool IsWideString(string target);
    static string CombineStringVector(vector<string> target);
    static void MakeBoolean(bool origin, string &target);

    static bool IsDigit(char c) { 
      return (c >= '0' && c <= '9'); 
    }

    static bool IsAlpha(char c) { 
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); 
    }
  };
}