#pragma once

#include <fstream>
#include "object.h"
#include "message.h"
#include "entry.h"

#if defined(_WIN32)
#if defined(_MSC_VER)
#pragma execution_character_set("utf-8")
#endif
#include "windows.h"
#define WIN32_LEAN_AND_MEAN
#else
#include <dlfcn.h>
#endif

namespace kagami {
  /*Processor Class
  The most important part of script processor.Original string will be tokenized and
  parsed here.Processed data will be delivered to entry provider.
  */
  class Processor {
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
  public:
    Processor() : health(false), commaExpFunc(false), insertBtnSymbols(false), disableSetEntry(false),
      dotOperator(false), defineLine(false), functionLine(false), subscriptProcessing(false),
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
}


