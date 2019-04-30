#pragma once
#include "ir.h"

namespace kagami {
  const map<string, string> kBracketPairs = {
    pair<string,string>(")", "("),
    pair<string,string>("]", "["),
    pair<string,string>("}", "{")
  };

  const vector<GenericToken> kReservedWordStore = {
    kTokenIf, kTokenElif, kTokenWhile, kTokenReturn,
    kTokenWhen, kTokenCase
  };

  const vector<GenericToken> kSingleWordStore = {
    kTokenEnd, kTokenElse, kTokenContinue, kTokenBreak
  };

  struct AnalyzerWorkBlock {
    deque<Argument> args;
    deque<Request> symbol;
    bool need_reversing;
    bool fn_line;
    bool foreach_line;
    bool local_object;
    Token current;
    Token next;
    Token next_2;
    Token last;
    Argument domain;
    int forward_priority;

    AnalyzerWorkBlock() :
      args(),
      symbol(),
      need_reversing(false),
      fn_line(false),
      foreach_line(false),
      local_object(false),
      current(),
      next(),
      next_2(),
      last(),
      domain(),
      forward_priority(0) {}
  };

  class Analyzer {
    vector<Token> tokens_;
    size_t index_;
    VMCode action_base_;
    string error_string_;

    vector<string> Scanning(string target);
    Message Tokenizer(vector<string> target);

    void ProduceVMCode(AnalyzerWorkBlock *blk);
    void Assign(AnalyzerWorkBlock *blk);
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
    Analyzer() : index_(0) {  }
    Analyzer(size_t index) : index_(index) {  }
    VMCode GetOutput() const { return action_base_; }

    GenericToken GetASTRoot() {
      if (action_base_.empty()) {
        return kTokenNull;
      }
      return action_base_.back().first.head_command;
    }

    void Clear();
    Message Make(string target, size_t index);
  };
}
