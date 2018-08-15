#pragma once
#pragma execution_character_set("utf-8")
#define _ENABLE_FASTRING_

#include <fstream>
#include "common.h"
#include "kit.h"
#include "object.h"
#include "message.h"

#if defined(_WIN32)
#include "windows.h"
#define WIN32_LEAN_AND_MEAN
#else
#include <dlfcn.h>
#endif

namespace kagami {
  using Token = pair<string, TokenTypeEnum>;

  const string kStrNormalArrow = ">>>";
  const string kStrDotGroup    = "...";
  const string kStrHostArgHead = "arg";
  const string kStrNop         = "nop";
  const string kStrDef         = "def";
  const string kStrRef         = "__ref";
  const string kStrCodeSub     = "__code_sub";
  const string kStrSub         = "__sub";
  const string kStrBinOp       = "BinOp";
  const string kStrIf          = "if";
  const string kStrElif        = "elif";
  const string kStrEnd         = "end";
  const string kStrElse        = "else";
  const string kStrVar         = "var";
  const string kStrSet         = "__set";
  const string kStrWhile       = "while";
  const string kStrFor         = "for";
  const string kStrLeftSelfInc  = "lSelfInc";
  const string kStrLeftSelfDec  = "lSelfDec";
  const string kStrRightSelfInc = "rSelfInc";
  const string kStrRightSelfDec = "rSelfDec";

  enum GenericTokenEnum {
    BG_NOP, BG_DEF, BG_REF, BG_CODE_SUB, 
    BG_SUB, BG_BINOP, BG_IF, BG_ELIF, 
    BG_END, BG_ELSE, BG_VAR, BG_SET, 
    BG_WHILE, BG_FOR, BG_LSELF_INC, BG_LSELF_DEC, 
    BG_RSELF_INC, BG_RSELF_DEC,
    BG_NUL
  };

  enum BasicTokenEnum {
    TOKEN_EQUAL, TOKEN_COMMA, TOKEN_LEFT_SQRBRACKET, TOKEN_DOT, 
    TOKEN_COLON, TOKEN_LEFT_BRACKET, TOKEN_RIGHT_SQRBRACKET, TOKEN_RIGHT_BRACKET, 
    TOKEN_SELFOP, TOKEN_OTHERS
  };

  /*Processor Class
  The most important part of script processor.Original string will be tokenized and
  parsed here.Processed data will be delivered to entry provider.
  */
  class Processor {
  public:
    Processor() : health(false), commaExpFunc(false), insertBtnSymbols(false), dotOperator(false),
      disableSetEntry(false),  defineLine(false), functionLine(false), subscriptProcessing(false),
       mode(0), nextInsertSubscript(0), lambdaObjectCount(0), index(0) {}

    bool IsHealth() const { return health; }
    string GetErrorString() const { return errorString; }

    bool IsSelfObjectManagement() const {
      string front = origin.front().first;
      return (front == kStrFor || front == kStrDef);
    }

    Processor &SetIndex(size_t idx) {
      this->index = idx;
      return *this;
    }

    size_t GetIndex() const {
      return index;
    }

    Token GetFirstToken() const {
      return origin.front();
    }

    Message Start(size_t mode = kModeNormal);
    Processor &Build(string target);
  private:
    
    bool                health;
    vector<Token>       origin;
    map<string, Object> lambdamap;
    deque<Token>  item, symbol;
    bool          commaExpFunc, insertBtnSymbols, disableSetEntry, dotOperator,
      defineLine, functionLine, subscriptProcessing;
    Token         currentToken;
    Token         nextToken;
    Token         forwardToken;
    string        operatorTargetType;
    string        errorString;
    size_t        mode, nextInsertSubscript, lambdaObjectCount, index;

    void   EqualMark();
    void   Comma();
    bool   LeftBracket(Message &msg);
    bool   RightBracket(Message &msg);
    void   LeftSquareBracket();
    bool   RightSquareBracket(Message &msg);
    void   OtherSymbols();
    bool   FunctionAndObject(Message &msg);
    void   OtherTokens();
    void   FinalProcessing(Message &msg);
    bool   SelfOperator(Message &msg);
    bool   Colon();
    Object *GetObj(string name);
    static vector<string> Spilt(string target);
    static string GetHead(string target);
    static int GetPriority(string target);
    bool   Assemble(Message &msg);
  };

  /*ScriptMachine class
  Script provider caches original string data from script file.
  */
  class ScriptMachine {
    std::ifstream stream;
    size_t current;
    vector<Processor> storage;
    vector<string> parameters;
    bool end;
    stack<size_t> cycleNestStack, cycleTailStack, modeStack;
    stack<bool> conditionStack;
    size_t currentMode;
    int nestHeadCount;
    bool health;
    bool isTerminal;

    void ConditionRoot(bool value);
    void ConditionBranch(bool value);
    void ConditionLeaf();
    void HeadSign(bool value, bool selfObjectManagement);
    void TailSign();

    void AddLoader(string raw) { 
      storage.push_back(Processor().Build(raw)); 
    }

    static bool IsBlankStr(string target) {
      if (target == kStrEmpty || target.size() == 0) return true;
      for (const auto unit : target) {
        if (unit != ' ' || unit != '\n' || unit != '\t' || unit != '\r') {
          return false;
        }
      }
      return true;
    }
  public:
    ~ScriptMachine() {
      stream.close();
      Kit().CleanupVector(storage).CleanupVector(parameters);
    }

    bool GetHealth() const { return health; }
    bool Eof() const { return end; }
    void ResetCounter() { current = 0; }
    ScriptMachine() { isTerminal = true; }
    explicit ScriptMachine(const char *target);
    Message Run();
    void Terminal();
  };

  /*Entry Class
  contains function pointer.Processed argument tokens are used for building
  new argument map.entry provider have two mode:internal function and plugin
  function.
  */
  class Entry {
  public:
    Entry() : id(kStrNull), priority(0), activity(nullptr), minsize(0) {
      parmMode = kCodeIllegalParm;
      specifictype = kTypeIdNull;
    }

    Entry(string id, Activity activity, int priority, int parmMode, string args, string type = kTypeIdNull) : 
    id(id), priority(priority), parmMode(parmMode), args(Kit().BuildStringVector(args)),
    specifictype(type) {
      this->activity = activity;
    }


    bool operator==(Entry &target) const {
      return (target.id    == this->id &&
        target.activity    == this->activity &&
        target.parmMode    == this->parmMode &&
        target.priority    == this->priority &&
        this->specifictype == target.specifictype &&
        target.args        == this->args);
    }

    Entry &SetSpecificType(string type) {
      this->specifictype = type;
      return *this;
    }

    string GetSpecificType()      const { return specifictype; }
    string GetId()                const { return this->id; }
    int GetArgumentMode()         const { return this->parmMode; }
    vector<string> GetArguments() const { return args; }
    size_t GetParameterSIze()     const { return this->args.size(); }
    int GetPriority()             const { return this->priority; }
    bool Good()                   const { return ((activity != nullptr) && parmMode != kCodeIllegalParm); }
    Message Start(ObjectMap &map) const;
  private:
    string id;
    int parmMode;
    int priority;
    vector<string> args;
    Activity activity;
    string specifictype;
    size_t minsize;
  };

  inline string CastToString(shared_ptr<void> ptr) {
    return *static_pointer_cast<string>(ptr);
  }

  void Activiate();
  void InitTemplates();
  void InitMethods();

  namespace type {
    ObjectPlanner *GetPlanner(string name);
    void AddTemplate(string name, ObjectPlanner temp);
    shared_ptr<void> GetObjectCopy(Object &object);
  }

  namespace trace {
    using log_t = pair<string, Message>;
    void Log(kagami::Message msg);
    vector<log_t> &GetLogger();
  }

  namespace management {
    enum OperatorCode {
      ADD, SUB, MUL, DIV, EQUAL, IS, NOT,
      MORE, LESS, NOT_EQUAL, MORE_OR_EQUAL, LESS_OR_EQUAL,
      SELFINC, SELFDEC,
      NUL
    };

    OperatorCode GetOperatorCode(string src);

    using EntryMapUnit = map<string, Entry>::value_type;

    list<ObjectManager> &GetObjectStack();
    ObjectManager       &GetCurrentManager();
    string              GetTypeId(string sign);
    void                Inject(Entry temp);
    void                LoadGenProvider(GenericTokenEnum token, Entry temp);
    Object              *FindObject(string name);
    ObjectManager       &CreateManager();
    bool                DisposeManager();
    Entry       Order(string id, string type, int size);
    std::wstring        s2ws(const std::string& s);
  }
}


