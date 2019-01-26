#pragma once
#include "message.h"
#include "object.h"

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
    TokenType token_type;
    Domain domain;

    Argument() :
      data(),
      type(AT_HOLDER),
      token_type(kTokenTypeNull) {

      domain.type = AT_HOLDER;
    }

    Argument(string data,
      ArgumentType type,
      TokenType token_type) :
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
    GenericToken head_gen;
    string head_reg;
    Argument domain;
    RequestType type;

    Request(GenericToken token) :
      priority(4),
      head_gen(token),
      head_reg(),
      domain(),
      type(RT_MACHINE) {}

    Request(string token, bool place_holder = false) :
      priority(4),
      head_gen(kTokenNull),
      head_reg(token),
      domain(),
      type(place_holder ? RT_NUL : RT_REGULAR) {}

    Request() :
      priority(4),
      head_gen(kTokenNull),
      head_reg(),
      domain(),
      type(RT_NUL) {}
  };

  using Command = pair<Request, deque<Argument>>;

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