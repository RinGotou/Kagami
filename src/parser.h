#pragma once

#include <fstream>
#include "object.h"
#include "message.h"
#include "entry.h"

#if defined(_WIN32)
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
    bool health;
    vector<Token> origin;
    deque<Object> item;
    deque<Entry> symbol;
    bool insertBetweenObject, dotOperator, needReverse, 
      defineLine, functionLine, subscriptProcessing;
    Token currentToken;
    Token nextToken;
    Token forwardToken;
    string operatorTargetType;
    string errorString;
    size_t mode, nextInsertSubscript, lambdaObjectCount, index;

    bool TakeAction(Message &msg);
    static Object *GetObj(string name);
    void EqualMark();
    bool Colon();
    void LeftBracket(Message &msg);
    bool RightBracket(Message &msg);
    bool LeftSqrBracket(Message &msg);
    bool RightSqrBracket(Message &msg);
    bool FunctionAndObject(Message &msg);
    void OtherToken();
    void OtherSymbol();
    void FinalProcessing(Message &msg);
  public:
    Processor() : health(false), insertBetweenObject(false),
      dotOperator(false), defineLine(false), functionLine(false), subscriptProcessing(false),
      mode(0), nextInsertSubscript(0), lambdaObjectCount(0), index(0) {}

    bool IsSelfObjectManagement() const {
      string front = origin.front().first;
      return (front == kStrFor || front == kStrDef);
    }

    Processor &SetIndex(size_t idx) {
      this->index = idx;
      return *this;
    }

    Message Activiate(size_t mode = kModeNormal);
    Processor &Build(string target);

    size_t GetIndex() const { return index; }
    Token GetFirstToken() const { return origin.front(); }
    bool IsHealth() const { return health; }
    string GetErrorString() const { return errorString; }
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
    static bool IsBlankStr(string target);
  public:
    ScriptMachine() : current(0), end(false), currentMode(kModeNormal),
    nestHeadCount(0), health(false), isTerminal(true) {}
    ScriptMachine(vector<Processor> storage) : current(0), end(false), currentMode(kModeNormal),
      nestHeadCount(0), health(true), isTerminal(true) {
      this->storage = storage;
    }

    explicit ScriptMachine(const char *target);
    Message Run();
    void Terminal();

    bool GetHealth() const { return health; }
    bool Eof() const { return end; }
    void ResetCounter() {
      current = 0;
      end = false;
    }
  };

  void Activiate();
  void InitPlanners();

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


