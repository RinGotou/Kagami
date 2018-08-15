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

  struct ActivityTemplate;
  class Message;
  class ObjectPlanner;
  class Object;

  using ObjectMap = map<string, Object>;
  using Parameter = pair<string, Object>;
  using CopyCreator = shared_ptr<void>(*)(shared_ptr<void>);
  using CastFunc = pair<string, CopyCreator>;
  using Activity = Message(*)(ObjectMap &);
  using NamedObject = pair<string, Object>;

  const string kEngineVersion = "0.7";
  const string kInsideName = "Clover";
#if defined(_WIN32)
  const string kPlatformType = "Windows";
#else
  const string kPlatformType = "Linux";
#endif
  const string kEngineName = "Kagami";
  const string kEngineAuthor = "Suzu Nakamura and Contributor(s)";
  const string kCopyright = "Copyright(c) 2017-2018";

  const string kStrNull       = "null";
  const string kStrEmpty      = "";
  const string kStrFatalError = "__FATAL__";
  const string kStrWarning    = "__WARNING__";
  const string kStrSuccess    = "__SUCCESS__";
  const string kStrEOF        = "__EOF__";
  const string kStrPass       = "__PASS__";
  const string kStrRedirect   = "__*__";
  const string kStrTrue       = "true";
  const string kStrFalse      = "false";
  const string kStrOperator   = "__operator";
  const string kStrObject     = "__object";
  const string kMethodPrint   = "__print";

  const int kCodeAutoFill        = 14;
  const int kCodeNormalParm      = 13;
  const int kCodeFillingSign     = 12;
  const int kCodeReturn          = 11;
  const int kCodeConditionLeaf   = 10;
  const int kCodeConditionBranch = 9;
  const int kCodeConditionRoot   = 8;
  const int kCodeObject          = 7;
  const int kCodeTailSign        = 5;
  const int kCodeHeadSign        = 4;
  const int kCodeQuit            = 3;
  const int kCodeRedirect        = 2;
  const int kCodeNothing         = 1;
  const int kCodeSuccess         = 0;
  const int kCodeBrokenEntry     = -1;
  const int kCodeOverflow        = -2;
  const int kCodeIllegalParm     = -3;
  const int kCodeIllegalCall     = -4;
  const int kCodeIllegalSymbol   = -5;
  const int kCodeBadStream       = -6;
  const int kCodeBadExpression   = -7;

  const int kFlagCoreEntry      = 0;
  const int kFlagNormalEntry    = 1;
  const int kFlagOperatorEntry  = 2;
  const int kFlagMethod         = 3;

  enum TokenTypeEnum {
    T_GENERIC, T_STRING, T_INTEGER, T_DOUBLE,
    T_BOOLEAN, T_SYMBOL, T_BLANK, T_CHAR, T_NUL
  };

  const string kTypeIdNull      = "null";
  const string kTypeIdInt       = "int";
  const string kTypeIdRawString = "string";
  const string kTypeIdArrayBase = "deque";
  const string kTypeIdCubeBase  = "cube";
  const string kTypeIdRef       = "ref";

  const size_t kModeNormal        = 0;
  const size_t kModeNextCondition = 1;
  const size_t kModeCycle         = 2;
  const size_t kModeCycleJump     = 3;
  const size_t kModeCondition     = 4;

  const regex kPatternSymbol(R"(\+\+|--|==|<=|>=|!=|&&|\|\||[[:Punct:]])");
}

