#pragma once
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include <deque>
#include <regex>
#include <cstddef>
#include <stack>
#include <locale>
#include <codecvt>
#include <cstdlib>
#include <cctype>
#include <typeinfo>
#include <iostream>
#include <ctime>
#include <type_traits>

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

  const string kEngineVersion  = "1.5 'White'";
  const string kBackendVerison = "Hatsuki";
#if defined(_WIN32)
  const string kPlatformType   = "Windows";
#else
  const string kPlatformType   = "Linux";
#endif
  const string kEngineName     = "Kagami Project";
  const string kMaintainer     = "Suzu Nakamura";
  const string kCopyright      = "Copyright(c) 2017-2018";

  const string kStrNull        = "null",
    kStrRedirect               = "__*__",
    kStrTrue                   = "true",
    kStrFalse                  = "false",
    kStrObject                 = "__object",
    kMethodPrint               = "__print";

  enum StateCode {
    kCodeWhen = 18,
    kCodeCase = 17,
    kCodeBreak = 16,
    kCodeContinue = 15,
    kCodeAutoSize = 14,
    kCodeDefineSign = 13,
    kCodeAutoFill = 12,
    kCodeNormalParm = 11,
    kCodeHeadPlaceholder = 10,
    kCodeReturn = 9,
    kCodeConditionLeaf = 8,
    kCodeConditionBranch = 7,
    kCodeConditionRoot = 6,
    kCodeObject = 5,
    kCodeTailSign = 4,
    kCodeHeadSign = 3,
    kCodeQuit = 2,
    kCodeRedirect = 1,
    kCodeSuccess = 0,
    kCodeIllegalParm = -1,
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

  const int 
    kFlagNormalEntry = 0,
    kFlagMethod      = 1;

  const map<string, string> kBracketPairs = {
    pair<string,string>(")", "("),
    pair<string,string>("]", "["),
    pair<string,string>("}", "{")
  };

  enum TokenTypeEnum {
    T_GENERIC, T_STRING, T_INTEGER, T_FLOAT,
    T_BOOLEAN, T_SYMBOL, T_BLANK, T_CHAR, T_NUL
  };

  using Token = pair<string, TokenTypeEnum>;

  enum GenericTokenEnum {
    GT_NOP, GT_DEF, 
    GT_IF, GT_ELIF, GT_END, GT_ELSE, 
    GT_BIND, 
    GT_WHILE, GT_FOR, GT_LSELF_INC, GT_LSELF_DEC,
    GT_RSELF_INC, GT_RSELF_DEC,
    GT_ADD, GT_SUB, GT_MUL, GT_DIV, GT_IS, 
    GT_LESS_OR_EQUAL, GT_MORE_OR_EQUAL, GT_NOT_EQUAL,
    GT_MORE, GT_LESS, GT_RETURN,
    GT_AND, GT_OR, GT_NOT, GT_BIT_AND, GT_BIT_OR, 
    GT_ARRAY, GT_TYPE_ASSERT, GT_ASSERT_R,
    GT_CONTINUE, GT_BREAK, 
    GT_CASE, GT_WHEN, GT_TYPEID, GT_EXIST,
    GT_DIR, GT_QUIT,
    GT_NUL
  };

  enum BasicTokenEnum {
    TOKEN_EQUAL, TOKEN_COMMA, TOKEN_LEFT_SQRBRACKET, TOKEN_DOT,
    TOKEN_COLON, TOKEN_LEFT_BRACKET, TOKEN_RIGHT_SQRBRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_SELFOP, TOKEN_LEFT_CURBRACKET, TOKEN_RIGHT_CURBRACKET, 
    TOKEN_OTHERS
  };

  

  const string kTypeIdNull       = "Null";
  const string kTypeIdString     = "String";
  const string kTypeIdWideString = "WString";
  const string kTypeIdRawString  = "RawString";
  const string kTypeIdArrayBase  = "Array";
  const string kTypeIdInStream   = "Instream";
  const string kTypeIdOutStream  = "Outstream";
  const string kTypeIdRegex      = "Regex";
  const string kTypeIdRef        = "Ref";
  const string kTypeIdLib        = "Library";
  const string kTypeIdFunction   = "Function";

  const string kRawStringMethods  = "size|__at|__print";
  const string kArrayBaseMethods  = "size|__at|__print";
  const string kStringMethods     = "size|__at|__print|substr|to_wide";
  const string kWideStringMethods = "size|__at|__print|substr|to_byte";
  const string kInStreamMethods   = "get|good|getlines|close|eof";
  const string kOutStreamMethods  = "write|good|close";
  const string kRegexMethods      = "match";
  const string kFunctionMethods   = "id|call|parms";

  constexpr size_t kModeNormal        = 0;
  constexpr size_t kModeNextCondition = 1;
  constexpr size_t kModeCycle         = 2;
  constexpr size_t kModeCycleJump     = 3;
  constexpr size_t kModeCondition     = 4;
  constexpr size_t kModeDef           = 5;
  constexpr size_t kModeCase          = 6;
  constexpr size_t kModeCaseJump      = 7;
  constexpr size_t kModeClass         = 8;

  /*Generic Token*/
  const string
    kStrIf           = "if",
    kStrDef          = "def",
    kStrRef          = "__ref",
    kStrEnd          = "end",
    kStrVar          = "var",
    kStrSet          = "__set",
    kStrBind         = "__bind",
    kStrVaSize       = "__size",
    kStrFor          = "for",
    kStrElse         = "else",
    kStrElif         = "elif",
    kStrWhile        = "while",
    kStrContinue     = "continue",
    kStrBreak        = "break",
    kStrCase         = "case",
    kStrWhen         = "when",
    kStrLeftSelfInc  = "lSelfInc",
    kStrLeftSelfDec  = "lSelfDec",
    kStrRightSelfInc = "rSelfInc",
    kStrRightSelfDec = "rSelfDec",
    kStrReturn       = "return",
    kStrAdd          = "+",
    kStrSub          = "-",
    kStrMul          = "*",
    kStrDiv          = "/",
    kStrIs           = "==",
    kStrAnd          = "&&",
    kStrOr           = "||",
    kStrNot          = "!",
    kStrBitAnd       = "&",
    kStrBitOr        = "|",
    kStrLessOrEqual  = "<=",
    kStrMoreOrEqual  = ">=",
    kStrNotEqual     = "!=",
    kStrMore         = ">",
    kStrLess         = "<",
    kStrNop          = "__nop",
    kStrPlaceHolder  = "__ph",
    kStrUserFunc     = "__func",
    kStrRetValue     = "__ret",
    kStrStopSign     = "__stop",
    kStrArray        = "__array",
    kStrTypeAssert   = "__type_assert",
    kStrTypeAssertR  = "__type_assert_r",
    kStrTypeId       = "typeid",
    kStrDir          = "dir",
    kStrExist        = "exist",
    kStrQuit         = "quit";
}

