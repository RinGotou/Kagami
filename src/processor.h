#pragma once
#include "object.h"
#include "message.h"
#include "entry.h"

namespace kagami {
  /*Processor Class
  The most important part of script processor.Original string will be tokenized and
  parsed here.Processed data will be delivered to entry provider.
  */
  using Inst = pair<Entry, deque<Object>>;

  using ProcCtlBlk = struct {
    deque<Object> item;
    deque<Entry> symbol;
    bool insertBetweenObject, dotOperator, needReverse,
      defineLine, subscriptProcessing;
    Token currentToken;
    Token nextToken;
    Token forwardToken;
    string operatorTargetType;
    size_t mode, nextInsertSubscript, lambdaObjectCount;
  };

  class Processor {
    bool health;
    vector<Token> origin;
    vector<Inst> instBase;
    size_t index;
    string errorString;
    bool cached;

    void Reversing(ProcCtlBlk *blk);
    bool TakeAction(Message &msg, ProcCtlBlk *blk);
    static Object *GetObj(string name, ProcCtlBlk *blk);
    void EqualMark(ProcCtlBlk *blk);
    void LeftBracket(Message &msg, ProcCtlBlk *blk);
    bool RightBracket(Message &msg, ProcCtlBlk *blk);
    bool LeftSqrBracket(Message &msg, ProcCtlBlk *blk);
    bool SelfOperator(Message &msg, ProcCtlBlk *blk);
    bool LeftCurBracket(Message &msg, ProcCtlBlk *blk);
    bool RightCurBracket(Message &msg, ProcCtlBlk *blk);
    bool FunctionAndObject(Message &msg, ProcCtlBlk *blk);
    void OtherToken(ProcCtlBlk *blk);
    void OtherSymbol(ProcCtlBlk *blk);
    void FinalProcessing(Message &msg, ProcCtlBlk *blk);
    Message RunWithCache();
  public:
    Processor() : health(false), index(0), cached(false) {}
    Processor &SetIndex(size_t idx) {
      this->index = idx;
      return *this;
    }

    Message Activiate(size_t mode = kModeNormal);
    Processor &Build(string target);

    size_t GetIndex() const { return index; }
    Token GetFirstToken() const { return origin.front(); }
    bool IsHealth() const { return health; }
  };
}