#pragma once
#include "trace.h"
#include "filestream.h"

#define INVALID_TOKEN Token(string(), kStringTypeNull)

namespace kagami {
  using CombinedCodeline = pair<size_t, string>;
  using CombinedToken = pair<size_t, deque<Token>>;

  class LexicalFactory {
  private:
    StandardLogger *logger_;

  private:
    deque<CombinedToken> *dest_;

    void Scan(deque<string> &output, string target);
  public:
    LexicalFactory() = delete;
    LexicalFactory(deque<CombinedToken> &dest, StandardLogger *logger) : 
      dest_(&dest), logger_(logger) {}

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
    void DeliveringExpr();
    void DotExpr();
    void UnaryExpr();
    void FuncInvokingExpr();
    bool IndexExpr();
    bool ArrayExpr();
    void BinaryExpr();
    bool FnExpr();
    bool StructExpr();
    bool ForEachExpr();

    bool OtherExpressions();
    void LiteralValue();
    
    Message Parse();
  public:
    LineParser() : frame_(nullptr), index_(0) {}
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
    bool inside_struct_;
    stack<size_t> nest_;
    stack<size_t> nest_end_;
    stack<size_t> nest_origin_;
    stack<size_t> cycle_escaper_;
    stack<Keyword> nest_type_;
    stack<JumpListFrame> jump_stack_;
    list<CombinedCodeline> script_;
    deque<CombinedToken> tokens_;

  private:
    StandardLogger *logger_;
    bool is_logger_held_;

  private:
    bool ReadScript(list<CombinedCodeline> &dest);

  public:
    ~VMCodeFactory() { if (is_logger_held_) delete logger_; }
    VMCodeFactory() = delete;
    VMCodeFactory(string path, VMCode &dest, 
      string log, bool rtlog = false) :
      dest_(&dest), path_(path), logger_(), is_logger_held_(true) {
      logger_ = rtlog ?
        (StandardLogger *)new StandardRTLogger(log.data(), "a") :
        (StandardLogger *)new StandardCachedLogger(log.data(), "a");
    }
    VMCodeFactory(string path, VMCode &dest,
      StandardLogger *logger) :
      dest_(&dest), path_(path), logger_(logger), is_logger_held_(false) {}
    
    bool Start();
  };
}
