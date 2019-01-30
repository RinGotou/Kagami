#pragma once
#include "message.h"
#include "object.h"

namespace kagami {
  enum ArgumentType {
    kArgumentNormal, 
    kArgumentObjectPool, 
    kArgumentReturningStack, 
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

  class Argument {
  public:
    string data;
    ArgumentType type;
    TokenType token_type;
    Domain domain;

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
    int priority;
    GenericToken head_command;
    string head_interface;
    Argument domain;
    RequestType type;

    Request(GenericToken token) :
      priority(4),
      head_command(token),
      head_interface(),
      domain(),
      type(kRequestCommand) {}

    Request(string token, bool place_holder = false) :
      priority(4),
      head_command(kTokenNull),
      head_interface(token),
      domain(),
      type(place_holder ? kRequestNull : kRequestInterface) {}

    Request() :
      priority(4),
      head_command(kTokenNull),
      head_interface(),
      domain(),
      type(kRequestNull) {}
  };

  using ArgumentList = deque <Argument>;
  using Command = pair<Request, ArgumentList>;

  class IR {
  private:
    vector<Command> container_;
    size_t index_;
    Token main_token_;
  public:
    IR() :
      index_(0) {}

    IR(vector<Command> commands,
      size_t index = 0,
      Token main_token = Token()) :
      index_(index) {

      container_ = commands;
      this->main_token_ = main_token;
    }

    vector<Command> &GetContains() {
      return container_;
    }

    size_t GetIndex() const {
      return index_;
    }

    Token GetMainToken() const {
      return main_token_;
    }
  };

  /* Origin index and string data */
  using StringUnit = pair<size_t, string>;
}