#pragma once
#include "object.h"
#include "message.h"
#include "entry.h"

namespace kagami {
  enum ArgumentType {
    AT_NORMAL, AT_OBJECT, AT_RET, AT_HOLDER
  };

  enum RequestType {
    RT_MACHINE, RT_REGULAR, RT_NUL
  };

  struct Domain {
    string data;
    ArgumentType type;
  };

  class Argument {
  public:
    string data;
    ArgumentType type;
    TokenTypeEnum token_type;
    Domain domain;

    Argument() :
      data(),
      type(AT_HOLDER),
      token_type(T_NUL) {
    
      domain.type = AT_HOLDER;
    }

    Argument(string data,
      ArgumentType type,
      TokenTypeEnum token_type) :
      data(data),
      type(type),
      token_type(token_type) {

      this->domain.data = "";
      this->domain.type = AT_HOLDER;
    }

    bool IsPlaceholder() const {
      return type == AT_HOLDER;
    }
  };

  class Request {
  public:
    int priority;
    GenericTokenEnum head_gen;
    string head_reg;
    Argument domain;
    RequestType type;

    Request(GenericTokenEnum token) :
      priority(4),
      head_gen(token),
      head_reg(),
      domain(),
      type(RT_MACHINE) {}

    Request(string token, bool place_holder = false) :
      priority(4),
      head_gen(GT_NUL),
      head_reg(token),
      domain(),
      type(place_holder ? RT_NUL : RT_REGULAR) {}

    Request():
      priority(4),
      head_gen(GT_NUL),
      head_reg(),
      domain(),
      type(RT_NUL) {}
  };

  using Instruction = pair<Request, deque<Argument>>;

  struct AnalyzerWorkBlock {
    deque<Argument> args;
    deque<Request> symbol;
    bool insert_between_object, need_reversing, define_line;
    Token current;
    Token next;
    Token next_2;
    Token last;
    size_t mode, next_insert_index;
    Argument domain;
  };

  class Analyzer {
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
