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

#ifndef _NO_CUI_
#include <iostream>
#endif

//Enable all debugging feature and output info
#define _ENABLE_DEBUGGING_

#if defined(_WIN32)
#include "windows.h"
#define WIN32_LEAN_AND_MEAN
#pragma warning(disable:4996)
#else
#include <dlfcn.h>
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

  struct ActivityTemplate;
  class Message;
  class ObjectPlanner;
  class Object;

  using StateCde = pair<const char *, int>;
  using ObjectMap = map<string, Object>;
  using ObjectPair = pair<string, Object>;
  using Parameter = pair<string, Object>;
  using CopyCreator = shared_ptr<void>(*)(shared_ptr<void>);
  using CastFunc = pair<string, CopyCreator>;
  using Activity = Message(*)(ObjectMap &);
  using NamedObject = pair<string, Object>;
  

  const string kEngineVersion = "1.1";
  const string kCodeName = "Marionette";
#if defined(_WIN32)
  const string kPlatformType = "Windows";
#else
  const string kPlatformType = "Linux";
#endif
  const string kEngineName = "Kagami - Alternative Scripting Kit";
  const string kEngineAuthor = "Suzu Nakamura and Contributor(s)";
  const string kCopyright = "Copyright(c) 2017-2018";

  const string kStrNull = "null",
    kStrEmpty = "",
    kStrFatalError = "__FATAL__",
    kStrWarning = "__WARNING__",
    kStrSuccess = "__SUCCESS__",
    kStrEOF = "__EOF__",
    kStrPass = "__PASS__",
    kStrRedirect = "__*__",
    kStrTrue = "true",
    kStrFalse = "false",
    kStrOperator = "__operator",
    kStrObject = "__object",
    kMethodPrint = "__print";


  const map<const char *, int> kStateCode = {

  };
  const int kCodeSDLInfo = 17,
    kCodeAutoSize = 16,
    kCodeDefineSign = 15,
    kCodeAutoFill = 14,
    kCodeNormalParm = 13,
    kCodeHeadPlaceholder = 12,
    kCodeReturn = 11,
    kCodeConditionLeaf = 10,
    kCodeConditionBranch = 9,
    kCodeConditionRoot = 8,
    kCodeObject = 7,
    kCodeTailSign = 5,
    kCodeHeadSign = 4,
    kCodeQuit = 3,
    kCodeRedirect = 2,
    kCodeNothing = 1,
    kCodeSuccess = 0,
    kCodeBrokenEntry = -1,
    kCodeOverflow = -2,
    kCodeIllegalParm = -3,
    kCodeIllegalCall = -4,
    kCodeIllegalSymbol = -5,
    kCodeBadStream = -6,
    kCodeBadExpression = -7,
    kCodeSDLError = -8;

  const int kFlagCoreEntry = 0,
    kFlagNormalEntry = 1,
    kFlagOperatorEntry = 2,
    kFlagMethod = 3;

  const map<string, string> kBracketPairs = {
    pair<string,string>(")","("),
    pair<string,string>("]","["),
    pair<string,string>("}", "{")
  };

  enum TokenTypeEnum {
    T_GENERIC, T_STRING, T_INTEGER, T_DOUBLE,
    T_BOOLEAN, T_SYMBOL, T_BLANK, T_CHAR, T_NUL
  };

  using Token = pair<string, TokenTypeEnum>;

  enum GenericTokenEnum {
    GT_NOP, GT_DEF, GT_REF, GT_CODE_SUB,
    GT_IF, GT_ELIF, GT_END, GT_ELSE, 
    GT_VAR, GT_SET, GT_BIND, 
    GT_WHILE, GT_FOR, GT_LSELF_INC, GT_LSELF_DEC,
    GT_RSELF_INC, GT_RSELF_DEC,
    GT_ADD, GT_SUB, GT_MUL, GT_DIV, GT_IS, 
    GT_LESS_OR_EQUAL, GT_MORE_OR_EQUAL, GT_NOT_EQUAL,
    GT_MORE, GT_LESS, GT_RETURN,
    GT_AND, GT_OR, GT_NOT, GT_BIT_AND, GT_BIT_OR, 
    GT_ARRAY,
    GT_NUL
  };

  enum BasicTokenEnum {
    TOKEN_EQUAL, TOKEN_COMMA, TOKEN_LEFT_SQRBRACKET, TOKEN_DOT,
    TOKEN_COLON, TOKEN_LEFT_BRACKET, TOKEN_RIGHT_SQRBRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_SELFOP, TOKEN_LEFT_CURBRACKET, TOKEN_RIGHT_CURBRACKET, 
    TOKEN_OTHERS
  };

  const string kRawStringMethods = "size|__at|__print";

  const string kTypeIdNull      = "Null";
  //const string kTypeIdInt       = "int";
  const string kTypeIdString    = "String";
  const string kTypeIdWideString = "WString";
  const string kTypeIdRawString = "RawString";
  const string kTypeIdArrayBase = "Array";
  const string kTypeIdInStream  = "instream";
  const string kTypeIdOutStream = "outstream";
  const string kTypeIdRegex     = "regex";
  //const string kTypeIdCubeBase  = "Cube";
  const string kTypeIdRef       = "Ref";

  const size_t kModeNormal        = 0;
  const size_t kModeNextCondition = 1;
  const size_t kModeCycle         = 2;
  const size_t kModeCycleJump     = 3;
  const size_t kModeCondition     = 4;
  const size_t kModeDef           = 5;

  /*Generic Token*/
  const string kStrIf = "if",
    kStrDef = "def",
    kStrRef = "__ref",
    kStrEnd = "end",
    kStrVar = "var",
    kStrSet = "__set",
    kStrBind = "__bind",
    kStrFor = "for",
    kStrElse = "else",
    kStrElif = "elif",
    kStrWhile = "while",
    kStrCodeSub = "__code_sub",
    kStrLeftSelfInc = "lSelfInc",
    kStrLeftSelfDec = "lSelfDec",
    kStrRightSelfInc = "rSelfInc",
    kStrRightSelfDec = "rSelfDec",
    kStrReturn = "return",
    kStrAdd = "+",
    kStrSub = "-",
    kStrMul = "*",
    kStrDiv = "/",
    kStrIs = "==",
    kStrAnd = "&&",
    kStrOr = "||",
    kStrNot = "!",
    kStrBitAnd = "&",
    kStrBitOr = "|",
    kStrLessOrEqual = "<=",
    kStrMoreOrEqual = ">=",
    kStrNotEqual = "!=",
    kStrMore = ">",
    kStrLess = "<",
    kStrNop = "__nop",
    kStrPlaceHolder = "__ph",
    kStrUserFunc = "__func",
    kStrRetValue = "__ret",
    kStrStopSign = "__stop",
    kStrArray = "__array";

  /*Prompt for terminal*/
  const string kStrNormalArrow = ">>>",
    kStrDotGroup = "...";

  const string kOpAdd = "__add",
    kOpSub = "__sub",
    kOpMul = "__mul",
    kOpDiv = "__div",
    kOpEqual = "__eq",
    kOpIs = "__is",
    kOpLessOrEqual = "__loeq",
    kOpMoreOrEqual = "__moeq",
    kOpNotEqual = "__neq",
    kOpNot = "__not",
    kOpLess = "__less",
    kOpMore = "__more",
    kOpLSelfInc = "__lsinc",
    kOpRSelfInc = "__rsinc",
    kOpLSelfDec = "__lsdec",
    kOpRSelfDec = "__rsdec";


  const regex kPatternSymbol(R"(\+\+|--|==|<=|>=|!=|&&|\|\||[[:Punct:]])");
}

