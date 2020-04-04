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
    kKeywordIncrease,
    kKeywordDecrease,
    kKeywordInitialArray,
    kKeywordContinue,
    kKeywordBreak,
    kKeywordCase,
    kKeywordWhen,
    kKeywordTypeId,
    kKeywordExist,
    kKeywordMethods,
    kKeywordQuit,
    kKeywordHandle,
    kKeywordWait,
    kKeywordLeave,
    kKeywordUsing,
    kKeywordUsingTable,
    kKeywordApplyLayout,
    kKeywordOffensiveMode,
    kKeywordStruct,
    kKeywordModule,
    kKeywordDomainAssertCommand,
    kKeywordInclude, 
    kKeywordSuper,
    kKeywordIsBaseOf,
    kKeywordHasBehavior,
    kKeywordIsVariableParam,
    kKeywordIsOptionalParam,
    kKeywordOptionalParamRange,
    kKeywordIsSameCopy,
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
    kTerminatorStruct,
    kTerminatorModule,
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
    kStrModule         = "module",
    kStrInclude        = "include",
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
    kStrSwitchLine     = "!switch_line",
    kStrCaseObj        = "!case",
    kStrIteratorObj    = "!iterator",
    kStrContainerKeepAliveSlot = "!containter_keepalive",
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
    kStrUsing          = "using",
    kStrReuseLayout    = "reuse_layout",
    kStrUsingTable     = "using_table",
    kStrApplyLayout    = "apply_layout",
    kStrOffensiveMode  = "offensive_mode",
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
    kStrIncrease       = "+=",
    kStrDecrease       = "-=",
    kStrUserFunc       = "__func",
    kStrTypeId         = "typeid",
    kStrMethodsCmd     = "methods",
    kStrExist          = "exist",
    kStrSwap           = "swap",
    kStrTrue           = "true",
    kStrFalse          = "false",
    kStrCompare        = "__compare",
    kStrRightHandSide  = "__rhs",
    kStrLeftHandSide   = "__lhs",
    kStrTextureTable   = "__texture_table",
    kStrFontObjectHead = "__font_",
    kStrInitializer    = "initializer",
    kStrStructId       = "__struct_id",
    kStrSuperStruct    = "!super_struct",
    kStrSuperStructId  = "!super_struct_id", //deprecated?
    kStrSuperStructInitializer = "!super_initializer", //deprecated?
    kStrModuleList     = "!module_list",
    kStrSuper          = "super",
    kStrIsBaseOf       = "is_base_of",
    kStrHasBehavior    = "has_behavior",
    kStrIsVariableParam = "is_variable_param",
    kStrIsOptionalParam = "is_optional_param",
    kStrOptionalParamRange = "optional_param_range",
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
  string ReplaceInvalidChar(string source);
}