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
      current(),
      next(),
      next_2(),
      last(),
      domain(),
      forward_priority(0) {}
  };

  class Analyzer {
    bool health_;
    vector<Token> tokens_;
    size_t index_;
    KIR action_base_;
    string error_string_;

    vector<string> Scanning(string target);
    Message Tokenizer(vector<string> target);

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

    size_t get_index() const { return index_; }

    KIR GetOutput() const { return action_base_; }

    bool Good() const { return health_; }

    void Clear();
    Message Make(string target, size_t index = 0);
  };
}
