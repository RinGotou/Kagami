#pragma once
#include <ctime>
#include <cstdio>
#include <clocale>
#include <cstdlib>

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <deque>
#include <regex>
#include <stack>
#include <type_traits>
#include <functional>
#include <list>
#include <charconv>
#include <variant>
#include <filesystem>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#if defined(_MSC_VER)
#pragma warning(disable:4996)
#endif
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#if not defined(_DISABLE_SDL_)
#include "dawn/src/dawn.ui.h"
#include "dawn/src/dawn.sound.h"
#endif

#include "minatsuki.log/src/minatsuki.log.h"

//Switching Debugging Feature
//#define _DEBUG_

#define ENGINE_NAME     "Kagami Project"
#define INTERPRETER_VER "2.21"
#define CODENAME        "Cradle"
#define MAINTAINER      "Suzu Nakamura"
#define COPYRIGHT       "Copyright(c) 2019"

#define DEFAULT_STACK_DEPTH 10000
//#define DEFAULT_GIL_TICK 20

namespace kagami {
  using std::string;
  using std::pair;
  using std::vector;
  using std::map;
  using std::unordered_map;
  using std::deque;
  using std::shared_ptr;
  using std::unique_ptr;
  using std::static_pointer_cast;
  using std::dynamic_pointer_cast;
  using std::regex;
  using std::regex_match;
  using std::make_shared;
  using std::make_unique;
  using std::size_t;
  using std::stack;
  using std::to_string;
  using std::stod;
  using std::stol;
  using std::wstring;
  using std::list;
  using std::initializer_list;
  using std::is_same;
  using std::is_base_of;
  using std::from_chars;
  using std::to_chars;
  using std::variant;
  
  using namespace minatsuki;

  /* Application Info */
  const string kInterpreterVersion = INTERPRETER_VER;
  const string kCodeName           = CODENAME;
#if defined(_WIN32)
  const string kPlatformType   = "Windows Platform";
#else
  const string kPlatformType   = "Unix-like Platform";
#endif
  const string kEngineName     = ENGINE_NAME;
  const string kMaintainer     = MAINTAINER;
  const string kCopyright      = COPYRIGHT;

  enum StringType {
    kStringTypeIdentifier, 
    kStringTypeString, 
    kStringTypeInt, 
    kStringTypeFloat,
    kStringTypeBool, 
    kStringTypeSymbol, 
    kStringTypeBlank,
    kStringTypeNull
  };

  using Token = pair<string, StringType>;

  /* Reserved keywords mark/IR framework commands */
  enum Keyword {
    kKeywordLocal,
    kKeywordHash,
    kKeywordFor,
    kKeywordIn,
    kKeywordNullObj,
    kKeywordDestroy,
    kKeywordConvert,
    kKeywordRefCount,
    kKeywordTime,
    kKeywordVersion,
    kKeywordCodeName,
    kKeywordSwap,
    kKeywordExpList, 
    kKeywordFn, 
    kKeywordIf, 
    kKeywordElif, 
    kKeywordEnd, 
    kKeywordElse, 
    kKeywordBind, 
    kKeywordDeliver,
    kKeywordWhile, 
    kKeywordPlus, 
    kKeywordMinus, 
    kKeywordTimes, 
    kKeywordDivide, 
    kKeywordEquals, 
    kKeywordLessOrEqual, 
    kKeywordGreaterOrEqual, 
    kKeywordNotEqual,
    kKeywordGreater, 
    kKeywordLess, 
    kKeywordReturn,
    kKeywordAnd, 
    kKeywordOr, 
    kKeywordNot, 
    kKeywordInitialArray, 
    kKeywordContinue, 
    kKeywordBreak, 
    kKeywordCase, 
    kKeywordWhen, 
    kKeywordTypeId, 
    kKeywordExist,
    kKeywordDir, 
    kKeywordQuit,
    kKeywordNull
  };

  enum Terminator {
    kTerminatorAssign, 
    kTerminatorComma, 
    kTerminatorLeftBracket, 
    kTerminatorDot,
    kTerminatorLeftParen, 
    kTerminatorRightSqrBracket, 
    kTerminatorRightBracket,
    kTerminatorLeftBrace, 
    kTerminatorRightCurBracket, 
    kTerminatorMonoOperator,
    kTerminatorBinaryOperator,
    kTerminatorFn,
    kTerminatorFor,
    kTerminatorIn,
    kTerminatorArrow,
    kTerminatorNull
  };

  /* Plain Type Code */
  enum PlainType {
    kPlainInt     = 1, 
    kPlainFloat   = 2, 
    kPlainString  = 3, 
    kPlainBool    = 4, 
    kNotPlainType = -1
  };

  /* Embedded type identifier strings */
  const string kTypeIdNull            = "null";
  const string kTypeIdInt             = "int";
  const string kTypeIdFloat           = "float";
  const string kTypeIdBool            = "bool";
  const string kTypeIdString          = "string";
  const string kTypeIdWideString      = "wstring";
  const string kTypeIdArray           = "array";
  const string kTypeIdInStream        = "instream";
  const string kTypeIdOutStream       = "outstream";
  const string kTypeIdRegex           = "regex";
  const string kTypeIdFunction        = "function";
  const string kTypeIdIterator        = "iterator";
  const string kTypeIdPair            = "pair";
  const string kTypeIdTable           = "table";
  const string kTypeIdStruct          = "struct";

  const string
    kStrImpl           = "impl",
    kStrStruct         = "struct",
    kStrRootScope      = "!root",
    kStrLocal          = "local",
    kStrHash           = "hash",
    kStrIf             = "if",
    kStrFn             = "fn",
    kStrNullObj        = "null_obj",
    kStrDestroy        = "destroy",
    kStrConvert        = "convert",
    kStrGetStr         = "get_str",
    kStrRefCount       = "ref_count",
    kStrTime           = "time",
    kStrVersion        = "version",
    kStrCodeNameCmd    = "codename",
    kStrEnd            = "end",
    kStrPrint          = "print",
    kStrSwitchLine     = "!switch_line",
    kStrCaseObj        = "!case",
    kStrIteratorObj    = "!iterator",
    kStrCommentBegin   = "=begin",
    kStrCommentEnd     = "=end",
    kStrFor            = "for",
    kStrIn             = "in",
    kStrElse           = "else",
    kStrElif           = "elif",
    kStrWhile          = "while",
    kStrContinue       = "continue",
    kStrBreak          = "break",
    kStrCase           = "case",
    kStrWhen           = "when",
    kStrReturn         = "return",
    kStrOptional       = "optional",
    kStrVariable       = "variable",
    kStrHead           = "head",
    kStrTail           = "tail",
    kStrPlus           = "+",
    kStrMinus          = "-",
    kStrTimes          = "*",
    kStrDiv            = "/",
    kStrIs             = "==",
    kStrAnd            = "&&",
    kStrOr             = "||",
    kStrNot            = "!",
    kStrLessOrEqual    = "<=",
    kStrGreaterOrEqual = ">=",
    kStrNotEqual       = "!=",
    kStrGreater        = ">",
    kStrLess           = "<",
    kStrUserFunc       = "__func",
    kStrTypeId         = "typeid",
    kStrDir            = "dir",
    kStrExist          = "exist",
    kStrSwap           = "swap",
    kStrTrue           = "true",
    kStrFalse          = "false",
    kStrCompare        = "__compare",
    kStrRightHandSide  = "__rhs",
    kStrLeftHandSide   = "__lhs",
    kStrMe             = "me";

  template <class _Lhs, class... _Rhs>
  inline bool compare(_Lhs lhs, _Rhs... rhs) {
    return ((lhs == rhs) || ...);
  }

  template <class T>
  inline bool find_in_vector(T t, const vector<T> &&vec) {
    for (auto &unit : vec) {
      if (t == unit) {
        return true;
      }
    }

    return false;
  }

  template <class T>
  inline bool find_in_vector(T t, const vector<T> &vec) {
    for (auto &unit : vec) {
      if (t == unit) {
        return true;
      }
    }

    return false;
  }
}

