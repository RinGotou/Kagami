#include "analyzer.h"

namespace kagami {
  Terminator GetTerminatorCode(string src) {
    if (src == "=")   return kBasicTokenAssign;
    if (src == ",")   return kBasicTokenComma;
    if (src == "[")   return kBasicTokenLeftSqrBracket;
    if (src == ".")   return kBasicTokenDot;
    if (src == "(")   return kBasicTokenLeftBracket;
    if (src == "]")   return kBasicTokenRightSqrBracket;
    if (src == ")")   return kBasicTokenRightBracket;
    if (src == "{")   return kBasicTokenLeftCurBracket;
    if (src == "}")   return kBasicTokenRightCurBracket;
    if (src == "!")   return kBasicTokenMonoOperator;
    if (src == "~")   return kBasicTokenMonoOperator;
    return kBasicTokenOther;
  }


  vector<string> Analyzer::Scanning(string target) {
    string current_string, temp;
    bool string_processing = false;
    bool delay_suspending = false;
    bool delay_switching = false;
    bool escape_flag = false;
    bool disable_escape = false;
    char current = 0, next = 0, last = 0;
    vector<string> output;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = target[idx];

      (idx < target.size() - 1) ?
        next = target[idx + 1] :
        next = 0;

      if (delay_suspending) {
        string_processing = false;
        delay_suspending = false;
      }

      (string_processing && last == '\\' && !disable_escape) ?
        escape_flag = true :
        escape_flag = false;

      if (disable_escape) disable_escape = false;

      if (current == '\'' && !escape_flag) {
        if (!string_processing && util::GetTokenType(current_string) == kTokenTypeBlank) {
          current_string.clear();
        }

        string_processing ?
          delay_suspending = true :
          string_processing = true, delay_switching = true;
      }

      if (!string_processing || delay_switching) {
        temp = current_string;
        temp.append(1, current);

        auto type = util::GetTokenType(temp);
        if (type == kTokenTypeNull) {
          auto type = util::GetTokenType(current_string);
          switch (type) {
          case kTokenTypeBlank:
            current_string.clear();
            current_string.append(1, current);
            break;
          case kTokenTypeInt:
            if (current == '.' && util::IsDigit(next)) {
              current_string.append(1, current);
            }
            else {
              output.emplace_back(current_string);
              current_string.clear();
              current_string.append(1, current);
            }
            break;
          default:
            output.emplace_back(current_string);
            current_string.clear();
            current_string.append(1, current);
            break;
          }
        }
        else {
          if (type == kTokenTypeInt && (temp[0] == '+' || temp[0] == '-')) {
            output.emplace_back(string().append(1, temp[0]));
            current_string = temp.substr(1, temp.size() - 1);
          }
          else {
            current_string = temp;
          }
        }

        delay_switching ?
          delay_switching = false :
          delay_switching = delay_switching;
      }
      else {
        escape_flag ?
          current = util::GetEscapeChar(current) :
          current = current;
        if (current == '\\' && last == '\\') disable_escape = true;
        current_string.append(1, current);
      }

      last = target[idx];
    }

    if (util::GetTokenType(current_string) != kTokenTypeBlank) {
      output.emplace_back(current_string);
    }

    return output;
  }

  Message Analyzer::Tokenizer(vector<string> target) {
    bool negative_flag = false;
    stack<string> bracket_stack;
    Token current = Token("", kTokenTypeNull),
      next = Token("", kTokenTypeNull),
      last = Token("", kTokenTypeNull);
    Message msg;
    
    tokens_.clear();
    health_ = true;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = Token(target[idx], util::GetTokenType(target[idx]));
      (idx < target.size() - 1) ?
        next = Token(target[idx + 1], util::GetTokenType(target[idx + 1])) :
        next = Token("", kTokenTypeNull);

      if (current.second == kTokenTypeNull) {
        msg = Message(kCodeBadExpression,
          "Unknown token - " + current.first + ".",
          kStateError);
        break;
      }

      if (compare(current.first, { "+", "-" })) {
        if (compare(last.second, { kTokenTypeSymbol,kTokenTypeNull })
          && compare(next.second, { kTokenTypeInt,kTokenTypeFloat })) {
          negative_flag = true;
        }
      }

      if (compare(current.first, { "(", "[", "{" })) {
        bracket_stack.push(current.first);
      }

      if (compare(current.first, { ")", "]", "}" })) {
        if (!bracket_stack.empty() && bracket_stack.top() != kBracketPairs.at(current.first)) {
          msg = Message(kCodeBadExpression, "Left bracket is missing.", kStateError);
          break;
        }
        else {
          bracket_stack.pop();
        }
      }

      if (current.first == ",") {
        if (last.second == kTokenTypeSymbol &&
          !compare(last.first, { "]", ")", "}", "'" })) {
          msg = Message(kCodeBadExpression, "Illegal comma position.", kStateError);
          break;
        }
      }

      if (compare(current.first, { "+", "-" }) && !tokens_.empty()) {
        if (tokens_.back().first == current.first) {
          tokens_.back().first.append(current.first);
        }
        else {
          tokens_.emplace_back(current);
        }
      }

      else if (negative_flag) {
        Token res = Token(last.first + current.first, current.second);
        tokens_.back() = res;
        negative_flag = false;
      }
      else {
        tokens_.emplace_back(current);
      }

      last = current;
    }

    return msg;
  }

  void Analyzer::Reversing(AnalyzerWorkBlock *blk) {
    if (blk->symbol.empty()) return;
    if (blk->symbol.size() == 1) return;

    deque<Request> temp_symbol;
    deque<Argument> temp_arg;

    while (blk->symbol.back().priority == blk->symbol[blk->symbol.size() - 2].priority) {
      temp_symbol.push_back(blk->symbol.back());

      temp_arg.push_back(blk->args.back());
      blk->symbol.pop_back();
      blk->args.pop_back();
    }
    temp_symbol.push_back(blk->symbol.back());
    blk->symbol.pop_back();

    for (int count = 2; count > 0; count -= 1) {
      temp_arg.push_back(blk->args.back());
      blk->args.pop_back();
    }

    while (!temp_symbol.empty()) {
      blk->symbol.push_back(temp_symbol.front());
      temp_symbol.pop_front();
    }
    while (!temp_arg.empty()) {
      blk->args.push_back(temp_arg.front());
      temp_arg.pop_front();
    }
  }

  bool Analyzer::InstructionFilling(AnalyzerWorkBlock *blk) {
    deque<Argument> arguments;
    size_t idx = 0, limit = 0;

    bool is_bin_operator = util::IsBinaryOperator(blk->symbol.back().head_command);
    bool is_mono_operator = util::IsMonoOperator(blk->symbol.back().head_command);
    bool reversed = (is_bin_operator && blk->need_reversing);

    if (is_bin_operator) limit = 2;
    if (is_mono_operator) limit = 1;
    
    while (!blk->args.empty() && !blk->args.back().IsPlaceholder()) {
      if ((is_bin_operator || is_mono_operator) && idx >= limit) break;

      reversed ?
        arguments.emplace_back(blk->args.back()) :
        arguments.emplace_front(blk->args.back());
      
      blk->args.pop_back();
      (is_bin_operator) ? idx += 1 : idx = idx;
    }

    if (!blk->args.empty()
      && blk->args.back().IsPlaceholder()
      && !is_bin_operator && !is_mono_operator) {
      
      blk->args.pop_back();
    }

    action_base_.emplace_back(Command(blk->symbol.back(), arguments));
    if (blk->symbol.back().option.forward_insertion) {
      blk->args.emplace_front(Argument("", kArgumentReturningStack, kTokenTypeNull));
    }
    else {
      blk->args.emplace_back(Argument("", kArgumentReturningStack, kTokenTypeNull));
    }

    blk->symbol.pop_back();
    return health_;
  }

  void Analyzer::EqualMark(AnalyzerWorkBlock *blk) {
    if (!blk->args.empty()) {
      Request request(kTokenBind);
      request.priority = util::GetTokenPriority(kTokenBind);
      blk->symbol.emplace_back(request);
    }
  }

  void Analyzer::Dot(AnalyzerWorkBlock *blk) {
    GenericToken token;

    (blk->next_2.first != "(" && blk->next_2.first != "[") ?
      token = kTokenAssertR :
      token = kTokenAssert;
    
    deque<Argument> arguments = {
      blk->args.back(),
      Argument(blk->next.first,kArgumentNormal,blk->next.second)
    };

    action_base_.emplace_back(Command(Request(token), arguments));
    blk->domain = blk->args.back();
    blk->args.pop_back();

    action_base_.back().first.option.no_feeding = true;
  }

  void Analyzer::MonoOperator(AnalyzerWorkBlock *blk) {
    auto token = util::GetGenericToken(blk->current.first);
    blk->symbol.emplace_back(Request(token));
  }

  void Analyzer::LeftBracket(AnalyzerWorkBlock *blk) {
    if (blk->fn_line) return;
    if (blk->last.second != TokenType::kTokenTypeGeneric) {
      blk->symbol.emplace_back(Request(kTokenExpList));
    }
    
    blk->symbol.push_back(Request(blk->current.first));
    blk->args.emplace_back(Argument());
  }

  bool Analyzer::RightBracket(AnalyzerWorkBlock *blk) {
    bool result = CleanupStack(blk);
    auto &symbol = blk->symbol;

    if (result) {
      if (blk->need_reversing) blk->need_reversing = false;

      if (compare(symbol.back().head_interface, { "(","[","{" })) {
        symbol.pop_back();
      }

      result = InstructionFilling(blk);
    }
    return result;
  }

  bool Analyzer::LeftSqrBracket(AnalyzerWorkBlock *blk) {
    bool result = true;

    deque<Argument> arguments = {
      blk->args.back(),
      Argument("__at", kArgumentNormal, kTokenTypeGeneric)
    };

    action_base_.emplace_back(Command(Request(kTokenAssert), arguments));

    Request request("__at");
    request.domain = blk->args.back();
    blk->symbol.emplace_back(request);
    blk->symbol.emplace_back(Request(blk->current.first, true));
    blk->args.pop_back();
    blk->args.emplace_back(Argument());

    return result;
  }

  bool Analyzer::LeftCurBracket(AnalyzerWorkBlock *blk) {
    bool result;
    if (blk->last.second == TokenType::kTokenTypeSymbol) {
      blk->symbol.emplace_back(Request(kTokenInitialArray));
      blk->symbol.emplace_back(Request(blk->current.first, true));
      blk->args.emplace_back(Argument());
      result = true;
    }
    else {
      result = false;
      error_string_ = "Illegal curly bracket location.";
    }
    return result;
  }

  bool Analyzer::FunctionAndObject(AnalyzerWorkBlock *blk) {
    GenericToken token = util::GetGenericToken(blk->current.first);
    auto token_type = util::GetTokenType(blk->current.first);
    auto token_next = util::GetGenericToken(blk->next.first);
    auto token_next_2 = util::GetGenericToken(blk->next_2.first);

    if (find_in_vector(token, kSingleWordStore)) {
      if (blk->next.second != kTokenTypeNull) {
        error_string_ = "Invalid syntax after " + blk->current.first;
        return false;
      }

      Request request(token);
      blk->symbol.emplace_back(request);
      return true;
    }

    if (blk->fn_line) {
      if (token_type != kTokenTypeGeneric) {
        error_string_ = "Invalid token in function definition";
        return false;
      }

      if (blk->current.first == kStrOptional || blk->current.first == kStrVaribale) {
        if (util::GetTokenType(blk->next.first) != kTokenTypeGeneric) {
          error_string_ = "Invalid syntax after " + blk->current.first;
          return false;
        }
      }

      blk->args.emplace_back(Argument(blk->current.first, kArgumentNormal, kTokenTypeGeneric));
      return true;
    }

    if (token == kTokenFn) {
      if (blk->next_2.first != "(") {
        error_string_ = "Invalid syntax after fn";
        return false;
      }

      blk->fn_line = true;
      //TODO: lambda function processing
      blk->symbol.emplace_back(Request(kTokenFn));
      blk->symbol.emplace_back(Request("(", true));
      blk->args.emplace_back(Argument());

      return true;
    }

    if (token == kTokenFor) {
      if (token_next_2 != kTokenIn) {
        error_string_ = "Invalid syntax after for";
        return false;
      }

      if (token_next != kTokenTypeGeneric) {
        error_string_ = "Invalid unit name after for";
        return false;
      }

      blk->foreach_line = true;
      blk->symbol.emplace_back(Request(kTokenFor));
      blk->args.emplace_back(Argument());
    }

    if (token == kTokenIn) {
      if (!blk->foreach_line) {
        error_string_ = "Invalid 'in' token";
        return false;
      }

      return true;
    }

    if (blk->foreach_line && token_next == kTokenIn) {
      blk->args.emplace_back(Argument(
        blk->current.first, kArgumentNormal, kTokenTypeGeneric));
      return true;
    }

    if (token != kTokenNull) {
      if (blk->next.first == "=" || util::IsOperator(token)) {
        error_string_ = "Trying to operate with reserved keyword";
        return false;
      }

      if (find_in_vector(token, kReservedWordStore)) {
        if (blk->next.first == "(") {
          error_string_ = "Invalid '(' after " + blk->current.first;
          return false;
        }

        blk->symbol.emplace_back(Request(token));
        blk->args.emplace_back(Argument());
        return true;
      }

      Request request(token);
      blk->symbol.emplace_back(request);

      return true;
    }


    if (blk->next.first == "(") {
      Request request(blk->current.first);
      if (blk->last.first == ".") {
        request.domain = blk->domain;
      }
      else {
        request.domain.type = kArgumentNull;
      }
      blk->symbol.emplace_back(request);

      return true;
    }

    if (blk->next.first == "=") {
      blk->args.emplace_back(Argument(
        blk->current.first, kArgumentNormal, kTokenTypeGeneric));
      return true;
    }

    Argument arg(blk->current.first, kArgumentObjectPool, kTokenTypeGeneric);
    if (blk->domain.type != kArgumentNull) {
      arg.domain.data = blk->domain.data;
      arg.domain.type = blk->domain.type;
    }
    blk->domain = Argument();
    blk->args.emplace_back(arg);
    
    return true;
  }

  void Analyzer::OtherToken(AnalyzerWorkBlock *blk) {
    blk->args.emplace_back(
      Argument(blk->current.first, kArgumentNormal, blk->current.second));
  }

  void Analyzer::OtherSymbol(AnalyzerWorkBlock *blk) {
    auto token = util::GetGenericToken(blk->current.first);
    int current_priority = util::GetTokenPriority(token);
    Request request(token);
    request.priority = current_priority;

    if (!blk->symbol.empty()) {
      bool stack_top_operator = util::IsBinaryOperator(blk->symbol.back().head_command);
      int stack_top_priority = util::GetTokenPriority(blk->symbol.back().head_command);

      while (!blk->symbol.empty() && stack_top_operator && stack_top_priority > current_priority) {
        Reversing(blk);
        if (!InstructionFilling(blk)) {
          health_ = false;
          error_string_ = "Operation error in binary operator.";
          break;
        }

        stack_top_operator = util::IsBinaryOperator(blk->symbol.back().head_command);
        stack_top_priority = util::GetTokenPriority(blk->symbol.back().head_command);
      }
    }

    blk->symbol.emplace_back(request);

    blk->forward_priority = current_priority;
  }

  void Analyzer::FinalProcessing(AnalyzerWorkBlock *blk) {
    bool checked = false;
    while (!blk->symbol.empty()) {
      if (blk->symbol.back().head_interface == "(") {
        error_string_ = "Right bracket is missing";
        health_ = false;
        break;
      }

      auto firstEnum = blk->symbol.back().head_command;
      if (blk->symbol.size() > 1 && util::IsBinaryOperator(firstEnum)) {
        if (util::IsBinaryOperator(blk->symbol[blk->symbol.size() - 2].head_command)) {
          if (checked) {
            checked = false;
          }
          else {
            checked = true;
            blk->need_reversing = true;
            Reversing(blk);
          }
        }
      }

      if (!util::IsBinaryOperator(blk->symbol.back().head_command)) {
        blk->need_reversing = false;
      }

      if (!InstructionFilling(blk)) break;
    }
  }

  bool Analyzer::CleanupStack(AnalyzerWorkBlock *blk) {
    string top_token = blk->symbol.empty()? "(" :
      blk->symbol.back().head_interface;
    bool checked = false;
    bool result = true;

    while (!blk->symbol.empty() && !compare(top_token, { "{","[","(" })
      && blk->symbol.back().head_command != kTokenBind) {

      auto first_token = blk->symbol.back().head_command;

      if (util::IsBinaryOperator(first_token)) {
        if (util::IsBinaryOperator(blk->symbol[blk->symbol.size() - 2].head_command)) {
          if (checked) {
            checked = false;
          }
          else {
            checked = true;
            blk->need_reversing = true;
            Reversing(blk);
          }
        }
      }

      result = InstructionFilling(blk);
      if (!result) break;
      top_token = blk->symbol.back().head_interface;
    }

    return result;
  }

  Message Analyzer::Parser() {
    auto state = true;
    const auto size = tokens_.size();
    Message result;

    AnalyzerWorkBlock *blk = new AnalyzerWorkBlock();

    for (size_t i = 0; i < size; ++i) {
      if (!health_) break;
      if (!state)  break;

      blk->current = tokens_[i];
      i + 1 < size ?
        blk->next = tokens_[i + 1] :
        blk->next = Token(string(), TokenType::kTokenTypeNull);
      i + 2 < size ?
        blk->next_2 = tokens_[i + 2] :
        blk->next_2 = Token(string(), TokenType::kTokenTypeNull);

      auto token_type = blk->current.second;
      if (token_type == kTokenTypeSymbol) {
        Terminator value = GetTerminatorCode(blk->current.first);
        switch (value) {
        case kBasicTokenAssign: EqualMark(blk); break;
        case kBasicTokenComma:          state = CleanupStack(blk); break;
        case kBasicTokenLeftSqrBracket: state = LeftSqrBracket(blk); break;
        case kBasicTokenDot:            Dot(blk); break;
        case kBasicTokenLeftBracket:    LeftBracket(blk); break;
        case kBasicTokenRightSqrBracket:state = RightBracket(blk); break;
        case kBasicTokenRightBracket:   state = RightBracket(blk); break;
        case kBasicTokenLeftCurBracket: state = LeftCurBracket(blk); break;
        case kBasicTokenRightCurBracket:state = RightBracket(blk); break;
        case kBasicTokenMonoOperator:   MonoOperator(blk); break;
        case kBasicTokenOther:          OtherSymbol(blk); break;
        default:break;
        }
      }
      else if (token_type == TokenType::kTokenTypeGeneric) {
        state = FunctionAndObject(blk);
      }
      else if (token_type == TokenType::kTokenTypeNull) {
        result = Message(kCodeIllegalParam, "Illegal token.", kStateError);
        state = false;
      }
      else OtherToken(blk);
      blk->last = blk->current;
    }

    if (state) {
      FinalProcessing(blk);
    }
    if (!state || !health_) {
      result = Message(kCodeBadExpression, error_string_, kStateError);
    }

    blk->args.clear();
    blk->args.shrink_to_fit();
    blk->symbol.clear();
    blk->symbol.shrink_to_fit();
    delete blk;
    return result;
  }

  Message Analyzer::Make(string target, size_t index) {
    vector<string> spilted_string = Scanning(target);
    Message msg = Tokenizer(spilted_string);

    this->index_ = index;
    if (msg.GetLevel() != kStateError) {
      msg = Parser();
    }

    if (!health_) return Message(kCodeBadExpression, error_string_, kStateError);

    Request request(kTokenSegment);
    ArgumentList args;
    
    args.emplace_back(
      Argument(to_string(index_), kArgumentNormal, kTokenTypeInt)
    );

    int code = static_cast<int>(util::GetGenericToken(tokens_.front().first));

    args.emplace_back(
      Argument(to_string(code), kArgumentNormal, kTokenTypeInt)
    );

    action_base_.emplace_front(std::make_pair(request, args));

    return msg;
  }
}
