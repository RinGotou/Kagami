#pragma once
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include <deque>
#include <regex>

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

  struct ActivityTemplate;
  class Message;
  class ObjTemplate;
  class Object;

  using ObjectMap      = map<string, Object>;
  using Parameter      = pair<string, Object>;
  using CopyCreator    = shared_ptr<void>(*)(shared_ptr<void>);
  using CastFunc       = pair<string, CopyCreator>;
  using Activity       = Message(*)(ObjectMap &);
  using CastAttachment = map<string, ObjTemplate> *(*)();
  using MemoryDeleter  = int(*)(void *);
  using Attachment     = vector<ActivityTemplate> * (*)();

#if defined(_WIN32)
  const string kEngineVersion = "0.7";
  const string kInsideName = "Clover";
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

  const size_t kGenericToken    = 0;
  const size_t kTypeString      = 1;
  const size_t kTypeInteger     = 2;
  const size_t kTypeDouble      = 3;
  const size_t kTypeBoolean     = 4;
  const size_t kTypeSymbol      = 5;
  const size_t kTypeBlank       = 6;
  const size_t kTypeChar        = 7;
  const size_t kTypeNull        = 100;

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

  //const regex kPatternGenericToken(R"([a-zA-Z_][a-zA-Z_0-9]*)");
  //const regex kPatternNumber(R"(\d+\.?\d*)");
  //const regex kPatternInteger(R"([-]?\d+)");
  //const regex kPatternDouble(R"([-]?\d+\.\d+)");
  //const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
  //const regex kPatternBlank(R"([[:blank:]])");
  const regex kPatternSymbol(R"(\+\+|--|==|<=|>=|!=|&&|\|\||[[:Punct:]])");

  /*Activity Template class
    no description yet.
  */
  struct ActivityTemplate {
    string id;
    Activity activity;
    int priority;
    int parmMode;
    string args;
    string specifictype;

    ActivityTemplate(string id, Activity activity, int priority, int arg_mode, string args, string type = kTypeIdNull) {
      this->id = id;
      this->activity = activity;
      this->priority = priority;
      this->parmMode = arg_mode;
      this->args = args;
      this->specifictype = type;
    }
    ActivityTemplate(){}
  };

  /* Object Template Class
  this class contains custom class info for script language.
  */
  class ObjTemplate {
  private:
    CopyCreator copyCreator;
    string methods;
  public:
    ObjTemplate() : methods(kStrEmpty) {
      copyCreator = nullptr;
    }
    ObjTemplate(CopyCreator copyCreator, string methods) {
      this->copyCreator = copyCreator;
      this->methods     = methods;
    }

    shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) const {
      shared_ptr<void> result = nullptr;
      if (target != nullptr) {
        result = copyCreator(target);
      }
      return result;
    }
    string GetMethods() const {
      return methods;
    }
  };

  /*Message Class
    It's the basic message tunnel of this script processor.
    According to my design,processor will check value or detail or
    both of them to find out warnings or errors.Some functions use
    value,detail and castpath to deliver Object class.
  */
  class Message {
  private:
    string value;
    string detail;
    int code;
    shared_ptr<void> object;
    size_t idx;
  public:
    Message() {
      value = kStrEmpty;
      code = kCodeSuccess;
      detail = kStrEmpty;
      idx = 0;
    }

    Message(string value, int code, string detail) {
      this->value  = value;
      this->code   = code;
      this->detail = detail;
    }

    Message &combo(string value, int code, string detail) {
      this->value  = value;
      this->code   = code;
      this->detail = detail;
      return *this;
    }

    Message &SetValue(const string &value) {
      this->value  = value;
      return *this;
    }

    Message &SetCode(const int &code) {
      this->code = code;
      return *this;
    }

    Message &SetDetail(const string &detail) {
      this->detail = detail;
      return *this;
    }

    Message &SetIndex(const size_t index) {
      idx = index;
      return *this;
    }

    string GetValue()  const  { return this->value; }
    int GetCode()      const  { return this->code; }
    string GetDetail() const  { return this->detail; }
    size_t GetIndex()  const  { return idx; }
    Object GetObj() const;
    void SetObject(Object &object, string id);
  };

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

    static string GetRawString(string target) {
      return target.substr(1, target.size() - 2);
    }

    static bool IsDigit(char c) { return (c >= '0' && c <= '9'); }
    static bool IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    static bool IsString(string target) { 
      if (target.empty()) return false;
      return(target.front() == '\'' && target.back() == '\''); 
    }
    static bool IsGenericToken(string target);
    static bool IsInteger(string target);
    static bool IsDouble(string target);
    static bool IsBlank(string target);
    static size_t GetDataType(string target);
    bool FindInStringGroup(string target, string source);
    static vector<string> BuildStringVector(string source);
    static char ConvertChar(char target);
    static wchar_t ConvertWideChar(wchar_t target);
    static bool IsWideString(string target);
  };

  /*Object Class
  A shared void pointer is packaged in this.Almost all variables and
  constants are managed by shared pointers.This class will be packaged
  in ObjectManager class.
  */
  class Object {
  private:
    typedef struct { Object *ptr; } TargetObject;
    std::shared_ptr<void> ptr;
    string option;
    string methods;
    size_t tokenType;
    bool ro;
    bool permanent;
  public:
    Object() {
      //hold a null pointer will cause some mysterious ploblems,
      //so this will hold a specific value intead of nullptr
      ptr = nullptr;
      option = kTypeIdNull;
      tokenType = kTypeNull;
      ro = false;
      permanent = false;
    }
    Object &Manage(string t, string option = kTypeIdRawString) {
      Object *result;
      if (this->option == kTypeIdRef) {
        result = &(static_pointer_cast<TargetObject>(this->ptr)
          ->ptr
          ->Manage(t, option));
      }
      else {
        this->ptr = std::make_shared<string>(t);
        this->option = option;
        result = this;
      }
      return *result;
    }
    Object &Set(shared_ptr<void> ptr, string option) {
      Object *result;
      if (this->option == kTypeIdRef) {
        result = &(static_pointer_cast<TargetObject>(this->ptr)
          ->ptr
          ->Set(ptr, option));
      }
      else {
        this->ptr = ptr;
        this->option = option;
        result = this;
      }
      return *result;
    }
    Object &Ref(Object &object) {
      this->option = kTypeIdRef;
      if (!object.IsRef()) {
        TargetObject target;
        target.ptr = &object;
        ptr = make_shared<TargetObject>(target);
      }
      else {
        this->ptr = object.ptr;
      }
      return *this;
    }
    shared_ptr<void> Get() const {
      shared_ptr<void> result = ptr;
      if (option == kTypeIdRef) {
        result = static_pointer_cast<TargetObject>(ptr)
          ->ptr
          ->Get();
      }
      return result;
    }
    string GetTypeId() const {
      string result = option;
      if (option == kTypeIdRef) {
        result = static_pointer_cast<TargetObject>(ptr)
          ->ptr
          ->GetTypeId();
      }
      return result;
    }
    Object &SetMethods(string methods) {
      if (option == kTypeIdRef) {
        return static_pointer_cast<TargetObject>(ptr)
          ->ptr
          ->SetMethods(methods);
      }
      this->methods = methods;
      return *this;
    }
    Object &SetTokenType(size_t tokenType) {
      if (option == kTypeIdRef) {
        return static_pointer_cast<TargetObject>(ptr)
          ->ptr
          ->SetTokenType(tokenType);
      }
      this->tokenType = tokenType;
      return *this;
    }
    Object &SetRo(bool ro) {
      if (option == kTypeIdRef) {
        return static_pointer_cast<TargetObject>(ptr)
          ->ptr
          ->SetRo(ro);
      }
      this->ro = ro;
      return *this;
    }
    Object &SetPermanent(bool permanent) {
      this->permanent = permanent;
      return *this;
    }
    string GetMethods() const {
      if (option == kTypeIdRef) {
        return static_pointer_cast<TargetObject>(ptr)
          ->ptr
          ->GetMethods();
      }
      return methods;
    }
    size_t GetTokenType() const {
      if (option == kTypeIdRef) {
        return static_pointer_cast<TargetObject>(ptr)
          ->ptr
          ->GetTokenType();
      }
      return tokenType;
    }
    bool IsRo() const {
      if (option == kTypeIdRef) {
        return static_pointer_cast<TargetObject>(ptr)
          ->ptr
          ->IsRo();
      }
      return ro;
    }
    bool IsPermanent() const {
      return permanent;
    }
    void Clear() {
      ptr = make_shared<int>(0);
      option = kTypeIdNull;
      methods.clear();
      tokenType = kTypeNull;
      ro = false;
      permanent = false;
    }
    bool IsRef() const {
      return option == kTypeIdRef;
    }
    bool operator==(Object &object) const {
      return (ptr == object.ptr &&
        option == object.option &&
        methods == object.methods &&
        tokenType == object.tokenType &&
        ro == object.ro);
    }
    bool operator!=(Object &object) const {
      return (ptr != object.ptr &&
        option != object.option &&
        methods != object.methods &&
        tokenType != object.tokenType &&
        ro != object.ro);
    }
    Object &Copy(Object &object) {
      ptr = object.ptr;
      option = object.option;
      methods = object.methods;
      tokenType = object.tokenType;
      ro = object.ro;
      return *this;
    }
    Object &Copy(Object &&object) {
      ptr = object.ptr;
      option = object.option;
      methods = object.methods;
      tokenType = object.tokenType;
      ro = object.ro;
      return *this;
    }
  };


}

