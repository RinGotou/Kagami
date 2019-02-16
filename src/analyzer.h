#pragma once
#include "ir.h"

namespace kagami {
  struct AnalyzerWorkBlock {
    deque<Argument> args;
    deque<Request> symbol;
    bool need_reversing, 
      define_line;
    Token current;
    Token next;
    Token next_2;
    Token last;
    size_t mode;
    Argument domain;
    int forward_priority;
  };

  class Analyzer {
    bool health_;
    vector<Token> tokens_;
    size_t index_;
    vector<Command> action_base_;
    string error_string_;

    vector<string> Scanning(string target);
    Message Tokenizer(vector<string> target);

    void Reversing(AnalyzerWorkBlock *blk);
    bool InstructionFilling(AnalyzerWorkBlock *blk);
    void EqualMark(AnalyzerWorkBlock *blk);
    void Dot(AnalyzerWorkBlock *blk);
    void MonoOperator(AnalyzerWorkBlock *blk);
    void LeftBracket(AnalyzerWorkBlock *blk);
    bool RightBracket(AnalyzerWorkBlock *blk);
    bool LeftSqrBracket(AnalyzerWorkBlock *blk);
    bool LeftCurBracket(AnalyzerWorkBlock *blk);
    bool FunctionAndObject(AnalyzerWorkBlock *blk);
    void OtherToken(AnalyzerWorkBlock *blk);
    void OtherSymbol(AnalyzerWorkBlock *blk);
    void FinalProcessing(AnalyzerWorkBlock *blk);
    bool CleanupStack(AnalyzerWorkBlock *blk);
    
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

    vector<Command> GetOutput() const { 
      return action_base_; 
    }

    bool Good() const { 
      return health_; 
    }

    void Clear() {
      tokens_.clear();
      tokens_.shrink_to_fit();
      action_base_.clear();
      action_base_.shrink_to_fit();
      health_ = false;
      error_string_.clear();
      index_ = 0;
    }

    Message Make(string target, size_t index = 0);
  };
}
