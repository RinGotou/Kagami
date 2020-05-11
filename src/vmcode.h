#pragma once
#include "message.h"
#include "object.h"

namespace kagami {
  enum ArgumentType {
    kArgumentLiteral, 
    kArgumentObjectStack, 
    kArgumentReturnStack, 
    kArgumentNull
  };

  enum RequestType {
    kRequestCommand, 
    kRequestFunction, 
    kRequestNull
  };

  struct ArgumentOption {
    bool optional_param;
    bool variable_param;
    bool use_last_assert;
    bool assert_chain_tail;

    string domain;
    ArgumentType domain_type;

    ArgumentOption() : 
      optional_param(false),
      variable_param(false),
      use_last_assert(false),
      assert_chain_tail(false),
      domain(), 
      domain_type(kArgumentNull) {}
  };

  struct RequestOption {
    bool void_call;
    bool local_object;
    bool ext_object;
    bool use_last_assert;
    size_t nest;
    size_t nest_end;
    size_t escape_depth;
    Keyword nest_root;

    RequestOption() : 
      void_call(false), 
      local_object(false), 
      ext_object(false),
      use_last_assert(false),
      nest(0),
      nest_end(0),
      escape_depth(0),
      nest_root(kKeywordNull) {}
  };

  class Argument {
  private:
    string data_;
    ArgumentType type_;
    StringType token_type_;

  public:
    ArgumentOption option;

  public:
    Argument() :
      data_(),
      type_(kArgumentNull),
      token_type_(kStringTypeNull),
      option() {}

    Argument(
      string data,
      ArgumentType type,
      StringType token_type) :
      data_(data),
      type_(type),
      token_type_(token_type),
      option() {}

    void SetDomain(string id, ArgumentType type) {
      option.domain = id;
      option.domain_type = type;
    }

    auto &GetData() { return data_; }
    auto &GetType() { return type_; }
    StringType GetStringType() { return token_type_; }
    bool IsPlaceholder() const { return type_ == kArgumentNull; }
  };

  struct FunctionInfo {
    string id;
    Argument domain;
  };

  class Request {
  private:
    variant<Keyword, FunctionInfo> data_;

  public:
    size_t idx;
    RequestType type;
    RequestOption option;

    Request(Keyword token) :
      data_(token),
      idx(0),
      type(kRequestCommand),
      option() {}

    Request(string token, Argument domain = Argument()) :
      data_(FunctionInfo{ token, domain }),
      idx(0),
      type(kRequestFunction),
      option() {}

    Request() :
      data_(),
      idx(0),
      type(kRequestNull),
      option() {}

    string GetInterfaceId() {
      if (type == kRequestFunction) {
        return std::get<FunctionInfo>(data_).id;
      }

      return string();
    }

    Argument GetInterfaceDomain() {
      if (type == kRequestFunction) {
        return std::get<FunctionInfo>(data_).domain;
      }

      return Argument();
    }

    Keyword GetKeywordValue() {
      if (type == kRequestCommand) {
        return std::get<Keyword>(data_);
      }

      return kKeywordNull;
    }

    bool IsPlaceholder() const {
      return type == kRequestNull;
    }
   };

  using ArgumentList = deque<Argument>;
  using Command = pair<Request, ArgumentList>;

  class VMCode : public deque<Command> {
  protected:
    VMCode *source_;
    unordered_map<size_t, list<size_t>> jump_record_;

  public:
    VMCode() : deque<Command>(), source_(nullptr) {}
    VMCode(VMCode *source) : deque<Command>(), source_(source) {}
    VMCode(const VMCode &rhs) : deque<Command>(rhs), source_(rhs.source_),
      jump_record_(rhs.jump_record_) {}
    VMCode(const VMCode &&rhs) : VMCode(rhs) {}

    void AddJumpRecord(size_t index, list<size_t> record) {
      jump_record_.emplace(make_pair(index, record));
    }

    bool FindJumpRecord(size_t index, stack<size_t> &dest);
  };

  using VMCodePointer = VMCode * ;
}