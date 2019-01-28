#pragma once
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include <deque>
#include <regex>
#include <stack>
#include <locale>
#include <codecvt>
#include <cstdlib>
#include <cctype>
#include <typeinfo>
#include <iostream>
#include <ctime>
#include <type_traits>
#include "shio/src/shio.h"

//if you build this project by cmake,please turn off all switch macro below.
// Disbale SDL2 componets for non-GUI environment
//#define _DISABLE_SDL_

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")
#if defined(_MSC_VER)
#pragma warning(disable:4996)
#endif
#else
#include <dlfcn.h>
#endif

#if not defined(_DISABLE_SDL_)
#if defined(_WIN32)
#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"SDL2main.lib")
#if defined(_DEBUG)
#pragma comment(lib,"SDL2test.lib")
#endif
#pragma comment(lib,"SDL2_image.lib")
#else
#endif
#include <SDL.h>
#include <SDL_image.h>
#endif

namespace kagami {
  using std::string;
  using std::pair;
  using std::vector;
  using std::map;
  using std::deque;
  using std::shared_ptr;
  using std::static_pointer_cast;
  using std::regex;
  using std::regex_match;
  using std::shared_ptr;
  using std::static_pointer_cast;
  using std::make_shared;
  using std::size_t;
  using std::ifstream;
  using std::ofstream;
  using std::stack;
  using std::to_string;
  using std::stoi;
  using std::stof;
  using std::stod;
  using std::wstring;
  using shio::any;
  using shio::any_cast;

  const string kInterpreterVersion  = "1.53";
  const string kIRFrameworkVersion = "August";
  const string kPatchName = "Kaleidoscope";
#if defined(_WIN32)
  const string kPlatformType   = "Windows";
#else
  const string kPlatformType   = "Linux";
#endif
  const string kEngineName     = "Kagami Project";
  const string kMaintainer     = "Suzu Nakamura";
  const string kCopyright      = "Copyright(c) 2017-2018";

  enum StateCode {
    kCodeWhen = 18,
    kCodeCase = 17,
    kCodeBreak = 16,
    kCodeContinue = 15,
    kCodeAutoSize = 14,
    kCodeDefineSign = 13,
    kCodeAutoFill = 12,
    kCodeNormalParam = 11,
    kCodeHeadPlaceholder = 10,
    kCodeReturn = 9,
    kCodeConditionLeaf = 8,
    kCodeConditionBranch = 7,
    kCodeConditionRoot = 6,
    kCodeObject = 5,
    kCodeTailSign = 4,
    kCodeHeadSign = 3,
    kCodeQuit = 2,
    kCodeSuccess = 0,
    kCodeIllegalParam = -1,
    kCodeIllegalCall = -2,
    kCodeIllegalSymbol = -3,
    kCodeBadStream = -4,
    kCodeBadExpression = -5
  };

  enum StateLevel {
    kStateNormal,
    kStateError,
    kStateWarning
  };

  const map<string, string> kBracketPairs = {
    pair<string,string>(")", "("),
    pair<string,string>("]", "["),
    pair<string,string>("}", "{")
  };

  enum TokenType {
    kTokenTypeGeneric, 
    kTokenTypeString, 
    kTokenTypeInt, 
    kTokenTypeFloat,
    kTokenTypeBool, 
    kTokenTypeSymbol, 
    kTokenTypeBlank,
    kTokenTypeNull
  };

  using Token = pair<string, TokenType>;

  enum GenericToken {
    kTokenNop, 
    kTokenFn, 
    kTokenIf, 
    kTokenElif, 
    kTokenEnd, 
    kTokenElse, 
    kTokenBind, 
    kTokenWhile, 
    kTokenPlus, 
    kTokenMinus, 
    kTokenTimes, 
    kTokenDivide, 
    kTokenEquals, 
    kTokenLessOrEqual, 
    kTokenGreaterOrEqual, 
    kTokenNotEqual,
    kTokenGreater, 
    kTokenLess, 
    kTokenReturn,
    kTokenAnd, 
    kTokenOr, 
    kTokenNot, 
    kTokenBitAnd, 
    kTokenBitOr, 
    kTokenInitialArray, 
    kTokenAssert, 
    kTokenAssertR,
    kTokenContinue, 
    kTokenBreak, 
    kTokenCase, 
    kTokenWhen, 
    kTokenTypeId, 
    kTokenExist,
    kTokenDir, 
    kTokenQuit,
    kTokenNull
  };

  enum BasicToken {
    kBasicTokenAssign, 
    kBasicTokenComma, 
    kBasicTokenLeftSqrBracket, 
    kBasicTokenDot,
    kBasicTokenLeftBracket, 
    kBasicTokenRightSqrBracket, 
    kBasicTokenRightBracket,
    kBasicTokenLeftCurBracket, 
    kBasicTokenRightCurBracket, 
    kBasicTokenOther
  };

  enum MachineMode {
    kModeNormal,
    kModeNextCondition,
    kModeCycle,
    kModeCycleJump,
    kModeCondition,
    kModeDef,
    kModeCase,
    kModeCaseJump
  };

  const string kTypeIdNull       = "null";
  const string kTypeIdString     = "string";
  const string kTypeIdWideString = "wstring";
  const string kTypeIdRawString  = "rawstring";
  const string kTypeIdArray      = "array";
  const string kTypeIdInStream   = "instream";
  const string kTypeIdOutStream  = "outstream";
  const string kTypeIdRegex      = "regex";
  const string kTypeIdFunction   = "function";

  const string
    kStrIf             = "if",
    kStrFn             = "fn",
    kStrEnd            = "end",
    kStrBind           = "__bind",
    kStrPrint          = "__print",
    kStrFor            = "for",
    kStrElse           = "else",
    kStrElif           = "elif",
    kStrWhile          = "while",
    kStrContinue       = "continue",
    kStrBreak          = "break",
    kStrCase           = "case",
    kStrWhen           = "when",
    kStrReturn         = "return",
    kStrPlus           = "+",
    kStrMinus          = "-",
    kStrTimes          = "*",
    kStrDiv            = "/",
    kStrIs             = "==",
    kStrAnd            = "&&",
    kStrOr             = "||",
    kStrNot            = "!",
    kStrBitAnd         = "&",
    kStrBitOr          = "|",
    kStrLessOrEqual    = "<=",
    kStrGreaterOrEqual = ">=",
    kStrNotEqual       = "!=",
    kStrGreater        = ">",
    kStrLess           = "<",
    kStrNop            = "__nop",
    kStrUserFunc       = "__func",
    kStrRetValue       = "__ret",
    kStrArray          = "__array",
    kStrTypeAssert     = "__type_assert",
    kStrTypeAssertR    = "__type_assert_r",
    kStrTypeId         = "typeid",
    kStrDir            = "dir",
    kStrExist          = "exist",
    kStrQuit           = "quit",
    kStrTrue           = "true",
    kStrFalse          = "false",
    kStrObject         = "__object";

  template <class Tx, class Ty>
  bool compare(Tx lhs, const std::initializer_list<Ty> &&rhs) {
    bool result = false;
    for (const auto &unit : rhs) {
      if (lhs == unit) result = true;
    }
    return result;
  }

  template <class T>
  bool find_in_vector(T t, vector<T> vec) {
    for (auto &unit : vec) {
      if (t == unit) {
        return true;
      }
    }

    return false;
  }
}

