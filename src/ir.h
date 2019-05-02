#pragma once
#include "message.h"
#include "object.h"

namespace kagami {
  enum ArgumentType {
    kArgumentNormal, 
    kArgumentObjectStack, 
    kArgumentReturnStack, 
    kArgumentNull
  };

  enum RequestType {
    kRequestCommand, 
    kRequestInterface, 
    kRequestNull
  };

  struct Domain {
    string data;
    ArgumentType type;
  };

  struct RequestOption {
    bool void_call;
    bool local_object;
    size_t nest;
    size_t nest_end;
    size_t escape_depth;
    Keyword nest_root;

    RequestOption() : 
      void_call(false), 
      local_object(false), 
      nest(0),
      nest_end(0),
      escape_depth(0),
      nest_root(kKeywordNull) {}
  };

  class Argument {
  public:
    string data;
    ArgumentType type;
    StringType token_type;
    Domain domain;

    Argument() :
      data(),
      type(kArgumentNull),
      token_type(kStringTypeNull) {

      domain.type = kArgumentNull;
    }

    Argument(string data,
      ArgumentType type,
      StringType token_type) :
      data(data),
      type(type),
      token_type(token_type) {

      this->domain.data = "";
      this->domain.type = kArgumentNull;
    }

    bool IsPlaceholder() const {
      return type == kArgumentNull;
    }
  };

  class Request {
  public:
    size_t idx;
    int priority;
    Keyword keyword_value;
    string interface_id;
    Argument domain;
    RequestType type;
    RequestOption option;

    Request(Keyword token) :
      idx(0),
      priority(5),
      keyword_value(token),
      interface_id(),
      domain(),
      type(kRequestCommand),
      option() {}

    Request(string token) :
      idx(0),
      priority(5),
      keyword_value(kKeywordNull),
      interface_id(token),
      domain(),
      type(kRequestInterface),
      option() {}

    Request() :
      idx(0),
      priority(5),
      keyword_value(kKeywordNull),
      interface_id(),
      domain(),
      type(kRequestNull),
      option() {}

    bool IsPlaceholder() const {
      return type == kRequestNull;
    }
   };

  using ArgumentList = deque<Argument>;
  using Command = pair<Request, ArgumentList>;

  class VMCode : public deque<Command> {
  protected:
    unordered_map<size_t, list<size_t>> jump_record_;

  public:
    void AddJumpRecord(size_t index, list<size_t> record) {
      jump_record_.emplace(std::make_pair(index, record));
    }

    bool FindJumpRecord(size_t index, stack<size_t> &dest);
  };

  using VMCodePointer = VMCode * ;
}