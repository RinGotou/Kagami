#pragma once
#include "object.h"
#include "message.h"
#include "entry.h"

namespace kagami {
  using Inst = pair<Entry, deque<Object>>;

  using ProcCtlBlk = struct {
    deque<Object> item;
    deque<Entry> symbol;
    bool insertBetweenObject, dotOperator, needReverse,
      defineLine, subscriptProcessing;
    Token currentToken;
    Token nextToken;
    Token forwardToken;
    stack<string> lastBracketStack;
    string operatorTargetType;
    size_t mode, nextInsertSubscript, lambdaObjectCount;
  };

  class Processor {
    bool health;
    vector<Inst> instBase;
    size_t index;
    Message Run();
    Token mainToken;
  public:
    Processor() : health(false), index(0) {}
    Processor(vector<Inst> instBase, size_t index = 0, Token mainToken = Token()) : health(true),
      index(index) {
      this->instBase = instBase;
      this->mainToken = mainToken;
    }
    
    Message Activiate(size_t mode = kModeNormal);
    size_t GetIndex() const { return index; }
    bool IsHealth() const { return health; }
  };
}