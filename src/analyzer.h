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
    vector<Token> tokens;
    size_t index;
    vector<Inst> instBase;
    string errorString;

    vector<string> Scanning(string target);
    Message Tokenizer(vector<string> target);

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
    Message Parser();
  public:
    Analyzer() :health(false), index(0) {  }
    Analyzer(size_t index) :health(false), index(index) {  }

    Token GetMainToken() const { 
      return tokens.front(); 
    }

    size_t GetIdx() const { 
      return index; 
    }
    vector<Inst> GetOutput() const { 
      return instBase; 
    }

    bool Good() const { 
      return health; 
    }

    void Clear();
    Message Make(string target, size_t index = 0);
  };
}
