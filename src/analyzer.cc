#include "analyzer.h"

#define INVALID_TOKEN Token(string(), kTokenTypeNull)
#define ERROR_MSG(_Msg) Message(kCodeBadExpression, _Msg, kStateError);

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
    string current_token, temp;
    bool inside_string = false;
    bool leave_string = false;
    bool enter_string = false;
    bool escape_flag = false;
    bool not_escape_char = false;
    char current = 0, next = 0, last = 0;
    vector<string> output;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = target[idx];

      (idx < target.size() - 1) ?
        next = target[idx + 1] :
        next = 0;

      if (leave_string) {
        inside_string = false;
        leave_string = false;
      }

      escape_flag = (inside_string && last == '\\' && !not_escape_char);

      if (not_escape_char) not_escape_char = false;

      if (current == '\'' && !escape_flag) {
        if (!inside_string && util::GetTokenType(current_token) == kTokenTypeBlank) {
          current_token.clear();
        }

        inside_string ?
          leave_string = true :
          inside_string = true, enter_string = true;
      }

      if (!inside_string || enter_string) {
        temp = current_token;
        temp.append(1, current);

        auto type = util::GetTokenType(temp);
        if (type == kTokenTypeNull) {
          auto type = util::GetTokenType(current_token);
          switch (type) {
          case kTokenTypeBlank:
            current_token.clear();
            current_token.append(1, current);
            break;
          case kTokenTypeInt:
            if (current == '.' && util::IsDigit(next)) {
              current_token.append(1, current);
            }
            else {
              output.emplace_back(current_token);
              current_token.clear();
              current_token.append(1, current);
            }
            break;
          default:
            output.emplace_back(current_token);
            current_token.clear();
            current_token.append(1, current);
            break;
          }
        }
        else {
          if (type == kTokenTypeInt && (temp[0] == '+' || temp[0] == '-')) {
            output.emplace_back(string().append(1, temp[0]));
            current_token = temp.substr(1, temp.size() - 1);
          }
          else {
            current_token = temp;
          }
        }


        if (enter_string) enter_string = false;
      }
      else {
        if (escape_flag) current = util::GetEscapeChar(current);
        if (current == '\\' && last == '\\') not_escape_char = true;
        current_token.append(1, current);
      }

      last = target[idx];
    }

    if (util::GetTokenType(current_token) != kTokenTypeBlank) {
      output.emplace_back(current_token);
    }

    return output;
  }

  Message Analyzer::Tokenizer(vector<string> target) {
    bool negative_flag = false;
    stack<string> bracket_stack;
    Token current = INVALID_TOKEN;
    Token next = INVALID_TOKEN;
    Token last = INVALID_TOKEN;
    Message msg;
    
    tokens_.clear();

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = Token(target[idx], util::GetTokenType(target[idx]));
      next = (idx < target.size() - 1) ?
        Token(target[idx + 1], util::GetTokenType(target[idx + 1])) :
        INVALID_TOKEN;

      if (current.second == kTokenTypeNull) {
        msg = ERROR_MSG("Unknown token - " + current.first);
        break;
      }

      if (compare_exp(current.first, "+", "-")) {
        if (compare_exp(last.second, kTokenTypeSymbol, kTokenTypeNull) && 
            compare_exp(next.second, kTokenTypeInt, kTokenTypeFloat)) {
          negative_flag = true;
          tokens_.push_back(current);
          last = current;
          continue;
        }
      }

      if (compare_exp(current.first, "(", "[", "{")) {
        bracket_stack.push(current.first);
      }

      if (compare_exp(current.first, ")", "]", "}")) {
        if (bracket_stack.empty()) {
          msg = ERROR_MSG("Left bracket is missing - " + current.first);
          break;
        }

        if (bracket_stack.top() != kBracketPairs.at(current.first)) {
          msg = ERROR_MSG("Left bracket is missing - " + current.first);
          break;
        }

        bracket_stack.pop();
      }

      if (current.first == ",") {
        if (last.second == kTokenTypeSymbol &&
          !compare_exp(last.first, "]", ")", "}", "'")) {
          msg = ERROR_MSG("Illegal comma position");
          break;
        }
      }


      if (compare_exp(last.first, "+", "-") && negative_flag) {
        Token combined(last.first + current.first, current.second);
        tokens_.back() = combined;
        negative_flag = false;
        last = combined;
        continue;
      }


      tokens_.emplace_back(current);
      last = current;
    }

    return msg;
  }

  void Analyzer::ProduceVMCode(AnalyzerWorkBlock *blk) {
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
    blk->symbol.pop_back();
    blk->args.emplace_back(Argument("", kArgumentReturnStack, kTokenTypeNull));
    if (blk->symbol.empty() && (blk->next.first == "," 
      || blk->next.second == kTokenTypeNull)) {
      action_base_.back().first.option.void_call = true;
    }
  }

  void Analyzer::Assign(AnalyzerWorkBlock *blk) {
    if (!blk->args.empty() && blk->next.first != kStrFn) {
      Request request(kTokenBind);
      request.option.local_object = blk->local_object;
      request.priority = util::GetTokenPriority(kTokenBind);
      blk->local_object = false;
      blk->symbol.emplace_back(request);
    }
  }

  void Analyzer::Dot(AnalyzerWorkBlock *blk) {
    blk->domain = blk->args.back();
    blk->args.pop_back();
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

      if (compare_exp(symbol.back().head_interface, "(", "[", "{")) {
        symbol.pop_back();
      }

      ProduceVMCode(blk);
    }
    return result;
  }

  bool Analyzer::LeftSqrBracket(AnalyzerWorkBlock *blk) {
    bool result = true;

    deque<Argument> arguments = {
      blk->args.back(),
      Argument("__at", kArgumentNormal, kTokenTypeGeneric)
    };

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
    bool lambda_func = false;

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

      if (blk->current.first == kStrOptional || blk->current.first == kStrVariable) {
        if (util::GetTokenType(blk->next.first) != kTokenTypeGeneric) {
          error_string_ = "Invalid syntax after " + blk->current.first;
          return false;
        }
      }

      blk->args.emplace_back(Argument(blk->current.first, kArgumentNormal, kTokenTypeGeneric));
      return true;
    }

    if (token == kTokenFn) {
      if (blk->next.first == "(" && blk->next.second == kTokenTypeGeneric) {
        lambda_func = true;
      }

      if (blk->next.first != "(" && blk->next_2.first != "(") {
        error_string_ = "Invalid syntax after fn";
        return false;
      }

      blk->fn_line = true;
      Request request(kTokenFn);
      //request.option.lambda_fn_obj = lambda_func;
      blk->symbol.emplace_back(request);
      blk->symbol.emplace_back(Request("(", true));
      blk->args.emplace_back(Argument());
      return true;
    }

    if (token == kTokenFor) {
      if (token_next_2 != kTokenIn) {
        error_string_ = "Invalid syntax after for";
        return false;
      }

      if (blk->next.second != kTokenTypeGeneric) {
        error_string_ = "Invalid unit name after for";
        return false;
      }

      blk->foreach_line = true;
      blk->symbol.emplace_back(Request(kTokenFor));
      blk->args.emplace_back(Argument());
      return true;
    }

    if (token == kTokenLocal) {
      if (blk->next_2.first != "=") {
        error_string_ = "Invalid 'local' token.";
        return false;
      }

      blk->local_object = true;
      return true;
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
        blk->symbol.emplace_back(Request(token));
        blk->args.emplace_back(Argument());
        return true;
      }
      else {
        if (blk->next.first != "(") {
          error_string_ = "Invalid syntax after " + blk->current.first;
          return false;
        }
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
      blk->domain = Argument();
      return true;
    }

    if (blk->next.first == "=") {
      blk->args.emplace_back(Argument(
        blk->current.first, kArgumentNormal, kTokenTypeGeneric));
      return true;
    }

    
    if (blk->domain.type != kArgumentNull) {
      Request request(blk->current.first);
      request.domain = blk->domain;
      blk->symbol.emplace_back(request);
      blk->args.emplace_back(Argument());
      ProduceVMCode(blk);
      blk->domain = Argument();
    }
    else {
      Argument arg(
        blk->current.first, kArgumentObjectStack, kTokenTypeGeneric);
      blk->args.emplace_back(arg);
    }
    
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

    //TODO:processing for same level
    if (!blk->symbol.empty()) {
      bool stack_top_operator = util::IsBinaryOperator(blk->symbol.back().head_command);
      int stack_top_priority = util::GetTokenPriority(blk->symbol.back().head_command);

      auto checking = [&stack_top_priority, &current_priority]()->bool {
        return (stack_top_priority >= current_priority);
      };

      while (!blk->symbol.empty() && stack_top_operator && checking()) {
        ProduceVMCode(blk);
        stack_top_operator = 
          (!blk->symbol.empty() && util::IsBinaryOperator(blk->symbol.back().head_command));
        stack_top_priority = blk->symbol.empty() ? 5 :
          util::GetTokenPriority(blk->symbol.back().head_command);
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
        break;
      }

      ProduceVMCode(blk);
    }
  }

  bool Analyzer::CleanupStack(AnalyzerWorkBlock *blk) {
    string top_token = blk->symbol.empty()? "(" :
      blk->symbol.back().head_interface;
    bool checked = false;
    bool result = true;

    while (!blk->symbol.empty() && !compare_exp(top_token, "{", "[", "(")
      && blk->symbol.back().head_command != kTokenBind) {
      ProduceVMCode(blk);
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
      if (!state)  break;

      blk->current = tokens_[i];
      blk->next = i + 1 < size ?
        tokens_[i + 1] : INVALID_TOKEN;
      blk->next_2 = i + 2 < size ?
        tokens_[i + 2] : INVALID_TOKEN;

      auto token_type = blk->current.second;
      if (token_type == kTokenTypeSymbol) {
        Terminator value = GetTerminatorCode(blk->current.first);
        switch (value) {
        case kBasicTokenAssign:         Assign(blk); break;
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
        result = ERROR_MSG("Illegal token - " + blk->current.first);
        state = false;
      }
      else OtherToken(blk);
      blk->last = blk->current;
    }

    if (state) FinalProcessing(blk);
    else result = ERROR_MSG(error_string_);

    for (auto &unit : action_base_) {
      unit.first.idx = index_;
    }

    blk->args.clear();
    blk->args.shrink_to_fit();
    blk->symbol.clear();
    blk->symbol.shrink_to_fit();
    delete blk;
    return result;
  }

  void Analyzer::Clear() {
    tokens_.clear();
    tokens_.shrink_to_fit();
    action_base_.clear();
    action_base_.shrink_to_fit();
    error_string_.clear();
    index_ = 0;
  }

  Message Analyzer::Make(string target, size_t index) {
    vector<string> spilted_string = Scanning(target);
    Message msg = Tokenizer(spilted_string);

    if (msg.GetLevel() == kStateError) return msg;

    index_ = index;
    msg = Parser();

    if (msg.GetLevel() == kStateError) return msg;

    Request request(kTokenSegment);
    ArgumentList args;

    int code = 
      static_cast<int>(util::GetGenericToken(tokens_.front().first));

    args.emplace_back(
      Argument(to_string(code), kArgumentNormal, kTokenTypeInt)
    );

    action_base_.emplace_front(std::make_pair(request, args));

    return msg;
  }
}
