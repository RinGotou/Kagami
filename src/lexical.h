#pragma once
#include "common.h"

namespace kagami {
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

  /* Reserved keywords mark / VMCode commands */
  enum Keyword {
    kKeywordAssert,
    kKeywordLocal,
    kKeywordExt,
    kKeywordHash,
    kKeywordFor,
    kKeywordIn,
    kKeywordNullObj,
    kKeywordDestroy,
    kKeywordConvert,
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
    kKeywordDelivering,
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
    kKeywordHandle,
    kKeywordWait,
    kKeywordLeave,
    kKeywordLoad,
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

  const string
    kStrAssert         = "assert",
    kStrThisWindow     = "this_window",
    kStrImpl           = "impl",
    kStrStruct         = "struct",
    kStrRootScope      = "!root",
    kStrLocal          = "local",
    kStrExt            = "ext",
    kStrHash           = "hash",
    kStrIf             = "if",
    kStrFn             = "fn",
    kStrNullObj        = "null_obj",
    kStrDestroy        = "destroy",
    kStrConvert        = "convert",
    kStrGetStr         = "get_str",
    kStrHandle         = "handle",
    kStrWait           = "wait",
    kStrLeave          = "leave",
    kStrTime           = "time",
    kStrVersion        = "version",
    kStrCodeNameCmd    = "codename",
    kStrEnd            = "end",
    kStrPrint          = "print",
    kStrLoad           = "load",
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
    kStrTextureTableHead   = "__texture_table_",
    kStrMe             = "me";

  wstring s2ws(const string &s);
  string ws2s(const wstring &s);
}

namespace kagami::lexical {
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
  string MakeBoolean(bool origin);
  bool IsDigit(char c);
  bool IsAlpha(char c);
  bool IsPlainType(string type_id);
  string ToUpper(string source);
  string ToLower(string source);
}