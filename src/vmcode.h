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

    Argument() :
      data(),
      type(kArgumentNull),
      token_type(kStringTypeNull) {}

    Argument(
      string data,
      ArgumentType type,
      StringType token_type) :
      data(data),
      type(type),
      token_type(token_type) {}

    bool IsPlaceholder() const {
      return type == kArgumentNull;
    }
  };

  struct InterfaceInfo {
    string id;
    Argument domain;
  };

  class Request {
  private:
    variant<Keyword, InterfaceInfo> data_;

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
      data_(InterfaceInfo{ token, domain }),
      idx(0),
      type(kRequestInterface),
      option() {}

    Request() :
      data_(),
      idx(0),
      type(kRequestNull),
      option() {}

    string GetInterfaceId() {
      if (type == kRequestInterface) {
        return std::get<InterfaceInfo>(data_).id;
      }

      return string();
    }

    Argument GetInterfaceDomain() {
      if (type == kRequestInterface) {
        return std::get<InterfaceInfo>(data_).domain;
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
    unordered_map<size_t, list<size_t>> jump_record_;

  public:
    void AddJumpRecord(size_t index, list<size_t> record) {
      jump_record_.emplace(std::make_pair(index, record));
    }

    bool FindJumpRecord(size_t index, stack<size_t> &dest);
  };

  using VMCodePointer = VMCode * ;
}