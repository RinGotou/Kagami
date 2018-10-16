#pragma once
#include "object.h"
#include "message.h"
#include "entry.h"

namespace kagami {
  enum ArgumentType {
    AT_NORMAL, AT_OBJECT, AT_RET, AT_HOLDER
  };

  class Argument {
  public:
    string data;
    ArgumentType type;
    TokenTypeEnum tokenType;

    Argument() :
      data(),
      type(AT_HOLDER),
      tokenType(T_NUL) {}

    Argument(string data, 
      ArgumentType type, 
      TokenTypeEnum tokenType) {
      this->data = data;
      this->type = type;
      this->tokenType = tokenType;
    }

    bool IsPlaceholder() const {
      return type == AT_HOLDER;
    }
  };

  using Instruction = pair<Entry, deque<Argument>>;

  class Analyzer {
    using AnalyzerWorkBlock = struct {
      deque<Argument> args;
      deque<Entry> symbol;
      bool insert_between_object, need_reversing, define_line;
      Token current;
      Token next;
      Token last;
      size_t mode, next_insert_index;
    };

    bool health_;
    vector<Token> tokens_;
    size_t index_;
    vector<Instruction> action_base_;
    string error_string_;

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
    Analyzer() :health_(false), index_(0) {  }
    Analyzer(size_t index) :health_(false), index_(index) {  }

    Token GetMainToken() const { 
      return tokens_.front(); 
    }

    size_t get_index() const { 
      return index_; 
    }

    vector<Instruction> GetOutput() const { 
      return action_base_; 
    }

    bool Good() const { 
      return health_; 
    }

    void Clear();
    Message Make(string target, size_t index = 0);
  };
}
