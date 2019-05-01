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

  struct ArgumentOption {
    bool void_call;
    bool local_object;
    bool segment_begin;
    size_t nest;
    size_t nest_end;
    Keyword segment_root;

    ArgumentOption() : 
      void_call(false), 
      local_object(false), 
      segment_begin(false),
      nest(0),
      nest_end(0),
      segment_root(kKeywordNull) {}
  };

  class Argument {
  public:
    string data;
    ArgumentType type;
    StringType token_type;
    Domain domain;
    ArgumentOption option;

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
    ArgumentOption option;

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
  using VMCode = deque<Command>;
  using VMCodePointer = VMCode * ;
}