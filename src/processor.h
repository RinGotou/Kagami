#pragma once
#include "object.h"
#include "message.h"
#include "entry.h"

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
    bool SelfOperator(Message &msg);
    bool FunctionAndObject(Message &msg);
    void OtherToken();
    void OtherSymbol();
    void FinalProcessing(Message &msg);
  public:
    Processor() : health(false), insertBetweenObject(false),
      dotOperator(false), defineLine(false), functionLine(false), subscriptProcessing(false),
      mode(0), nextInsertSubscript(0), lambdaObjectCount(0), index(0) {}

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
}