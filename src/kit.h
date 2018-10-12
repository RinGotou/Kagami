#pragma once
#include "common.h"

namespace kagami {
  /*Kit Class
  this class contains many useful template or tiny function, and
  create script processing workspace.
  */
  class Kit {
  public:
    template <class TypeA, class TypeB>
    Kit CleanupMap(map<TypeA, TypeB> &target) {
      using map_t = map<TypeA, TypeB>;
      target.clear();
      map_t(target).swap(target);
      return *this;
    }

    template <class Type>
    Kit CleanupVector(vector<Type> &target) {
      target.clear();
      vector<Type>(target).swap(target);
      return *this;
    }

    template <class Type>
    Kit CleanupDeque(deque<Type> &target) {
      target.clear();
      deque<Type>(target).swap(target);
      return *this;
    }

    template <class Type>
    bool Compare(Type source, vector<Type> list) {
      bool result = false;
      for (auto unit : list) {
        if (unit == source) {
          result = true;
          break;
        }
      }
      return result;
    }

    template <class Type>
    Type Calc(Type A, Type B, string opercode) {
      Type result = 0;
      if (opercode == "+") result = A + B;
      if (opercode == "-") result = A - B;
      if (opercode == "*") result = A * B;
      if (opercode == "/") result = A / B;
      return result;
    }

    template <class Type>
    static bool Logic(Type A, Type B, string opercode) {
      auto result = false;
      if (opercode == "==") result = (A == B);
      if (opercode == "<=") result = (A <= B);
      if (opercode == ">=") result = (A >= B);
      if (opercode == "!=") result = (A != B);
      if (opercode == ">")  result = (A > B);
      if (opercode == "<")  result = (A < B);
      return result;
    }

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
  };
}