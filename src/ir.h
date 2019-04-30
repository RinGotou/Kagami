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

    ArgumentOption() : 
      void_call(false), local_object(false) {}
  };

  class Argument {
  public:
    string data;
    ArgumentType type;
    TokenType token_type;
    Domain domain;
    ArgumentOption option;

    Argument() :
      data(),
      type(kArgumentNull),
      token_type(kTokenTypeNull) {

      domain.type = kArgumentNull;
    }

    Argument(string data,
      ArgumentType type,
      TokenType token_type) :
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
    GenericToken head_command;
    string head_interface;
    Argument domain;
    RequestType type;
    ArgumentOption option;

    Request(GenericToken token) :
      idx(0),
      priority(5),
      head_command(token),
      head_interface(),
      domain(),
      type(kRequestCommand),
      option() {}

    Request(string token, bool place_holder = false) :
      idx(0),
      priority(5),
      head_command(kTokenNull),
      head_interface(token),
      domain(),
      type(place_holder ? kRequestNull : kRequestInterface),
      option() {}

    Request() :
      idx(0),
      priority(5),
      head_command(kTokenNull),
      head_interface(),
      domain(),
      type(kRequestNull),
      option() {}
  };

  using ArgumentList = deque<Argument>;
  using Command = pair<Request, ArgumentList>;
  using VMCode = deque<Command>;
  using VMCodePointer = VMCode * ;
}