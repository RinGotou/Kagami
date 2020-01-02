#pragma once
#include "trace.h"
#include "filestream.h"

#define INVALID_TOKEN Token(string(), kStringTypeNull)

//Frontend version: hatsuki

namespace kagami {
  using CombinedCodeline = pair<size_t, string>;
  using CombinedToken = pair<size_t, deque<Token>>;

  class LexicalFactory {
  private:
    deque<CombinedToken> *dest_;

    void Scan(deque<string> &output, string target);
  public:
    LexicalFactory() = delete;
    LexicalFactory(deque<CombinedToken> &dest) : dest_(&dest) {}

    bool Feed(CombinedCodeline &src);

    auto &GetOutput() { return dest_; }
  };

  struct ParserFrame {
    deque<Argument> args;
    deque<Request> symbol;
    bool local_object;
    bool ext_object;
    bool eol;
    size_t idx;
    Token current;
    Token next;
    Token next_2;
    Token last;
    Argument domain;
    deque<Token> &tokens;

    ParserFrame(deque<Token> &tokens) :
      args(),
      symbol(),
      local_object(false),
      ext_object(false),
      eol(false),
      idx(0),
      current(INVALID_TOKEN),
      next(INVALID_TOKEN),
      next_2(INVALID_TOKEN),
      last(INVALID_TOKEN),
      domain(),
      tokens(tokens) {}

    void Eat();
  };

  class LineParser {
  private:
    ParserFrame *frame_;
    size_t index_;
    deque<Token> tokens_;
    VMCode action_base_;
    string error_string_;

    void ProduceVMCode();
    bool CleanupStack();

    void BindExpr();
    void DeliverExpr();
    void DotExpr();
    void UnaryExpr();
    void FuncInvokingExpr();
    bool IndexExpr();
    bool ArrayExpr();
    void BinaryExpr();
    bool FnExpr();
    bool ForEachExpr();

    bool OtherExpressions();
    void LiteralValue();
    
    Message Parse();
  public:
    LineParser() : index_(0) {}
    VMCode &GetOutput() { return action_base_; }

    Keyword GetASTRoot() {
      if (action_base_.empty()) {
        return kKeywordNull;
      }
      return action_base_.back().first.GetKeywordValue();
    }

    void Clear();
    Message Make(CombinedToken &line);
  };

  struct JumpListFrame {
    Keyword nest_code;
    size_t nest;
    list<size_t> jump_record;
  };

  class VMCodeFactory {
  private:
    VMCode *dest_;
    string path_;
    stack<size_t> nest_;
    stack<size_t> nest_end_;
    stack<size_t> nest_origin_;
    stack<size_t> cycle_escaper_;
    stack<Keyword> nest_type_;
    stack<JumpListFrame> jump_stack_;
    list<CombinedCodeline> script_;
    deque<CombinedToken> tokens_;

  private:
    bool ReadScript(list<CombinedCodeline> &dest);

  public:
    VMCodeFactory() = delete;
    VMCodeFactory(string path, VMCode &dest) :
      dest_(&dest), path_(path) {}
    
    bool Start();
  };
}
