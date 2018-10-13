#pragma once
#include "object.h"
#include "message.h"
#include "entry.h"

namespace kagami {
  enum ArgumentType {
    PT_NORMAL, PT_OBJ, PT_RET, PT_PHOLDER
  };

  class Argument {
  public:
    string data;
    ArgumentType type;
    TokenTypeEnum tokenType;

    Argument() :
      data(),
      type(PT_PHOLDER),
      tokenType(T_NUL) {}

    Argument(string data, 
      ArgumentType type, 
      TokenTypeEnum tokenType) {
      this->data = data;
      this->type = type;
      this->tokenType = tokenType;
    }

    bool IsPlaceholder() const {
      return type == PT_PHOLDER;
    }
  };

  using Instruction = pair<Entry, deque<Argument>>;

  using AnalyzerWorkBlock = struct {
    deque<Argument> args;
    deque<Entry> symbol;
    bool insertBetweenObject, needReverse, defineLine;
    Token currentToken;
    Token nextToken;
    Token forwardToken;
    size_t mode, nextInsertSubscript;
  };

  class Analyzer {
    bool health;
    vector<Token> tokens;
    size_t index;
    vector<Instruction> actionBase;
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

    vector<Instruction> GetOutput() const { 
      return actionBase; 
    }

    bool Good() const { 
      return health; 
    }

    void Clear();
    Message Make(string target, size_t index = 0);
  };
}
