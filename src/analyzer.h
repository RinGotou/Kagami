#pragma once
#include "object.h"
#include "message.h"
#include "entry.h"

namespace kagami {
  using Inst = pair<Entry, deque<Object>>;

  using AnalyzerWorkBlock = struct {
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

  class Analyzer {
    bool health;
    vector<Token> origin;
    size_t index;
    vector<Inst> instBase;
    string errorString;

    void Reversing(AnalyzerWorkBlock *blk);
    bool InstructionFilling(AnalyzerWorkBlock *blk);
    void EqualMark(AnalyzerWorkBlock *blk);
    void Dot(AnalyzerWorkBlock *blk);
    void LeftBracket(AnalyzerWorkBlock *blk);
    bool RightBracket(AnalyzerWorkBlock *blk);
    bool LeftSqrBracket(AnalyzerWorkBlock *blk);
    bool SelfOperator(AnalyzerWorkBlock *blk);
    bool LeftCurBracket(AnalyzerWorkBlock *blk);
    bool FunctionAndObject(AnalyzerWorkBlock *blk);
    void OtherToken(AnalyzerWorkBlock *blk);
    void OtherSymbol(AnalyzerWorkBlock *blk);
    void FinalProcessing(AnalyzerWorkBlock *blk);
    Message BuildTokens(string target);
    Message Analyze();
  public:
    Analyzer() :health(false), index(0) {  }
    Analyzer(size_t index) :health(false), index(index) {  }

    Message Make(string target,size_t index = 0) {
      Message msg = BuildTokens(target);
      this->index = index;
      if (msg.GetCode() >= kCodeSuccess) {
        msg = Analyze();
      }
      return msg;
    }

    Token GetMainToken() const { 
      return origin.front(); 
    }

    size_t GetIdx() const { return index; }
    vector<Inst> GetOutput() const { return instBase; }
    bool Good() const { return health; }
    void Clear();
  };
}
