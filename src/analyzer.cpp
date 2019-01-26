#include "analyzer.h"

namespace kagami {
  BasicToken GetBasicToken(string src) {
    if (src == "=")   return kBasicTokenAssign;
    if (src == ",")   return kBasicTokenComma;
    if (src == "[")   return kBasicTokenLeftSqrBracket;
    if (src == ".")   return kBasicTokenDot;
    if (src == "(")   return kBasicTokenLeftBracket;
    if (src == "]")   return kBasicTokenRightSqrBracket;
    if (src == ")")   return kBasicTokenRightBracket;
    if (src == "{")   return kBasicTokenLeftCurBracket;
    if (src == "}")   return kBasicTokenRightCurBracket;
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
    deque<Request> *temp_symbol = new deque<Request>();
    deque<Argument> *temp_arg = new deque<Argument>();

    while (!blk->symbol.empty() && blk->symbol.size() > 1
      && (blk->symbol.back().priority == blk->symbol[blk->symbol.size() - 2].priority)) {
      temp_symbol->push_back(blk->symbol.back());

      temp_arg->push_back(blk->args.back());
      blk->symbol.pop_back();
      blk->args.pop_back();
    }
    temp_symbol->push_back(blk->symbol.back());
    blk->symbol.pop_back();

    for (int count = 2; count > 0; count -= 1) {
      temp_arg->push_back(blk->args.back());
      blk->args.pop_back();
    }

    while (!temp_symbol->empty()) {
      blk->symbol.push_back(temp_symbol->front());
      temp_symbol->pop_front();
    }
    while (!temp_arg->empty()) {
      blk->args.push_back(temp_arg->front());
      temp_arg->pop_front();
    }
    
    delete temp_symbol;
    delete temp_arg;
  }

  bool Analyzer::InstructionFilling(AnalyzerWorkBlock *blk) {
    deque<Argument> arguments;
    size_t idx = 0, limit = 0;

    bool is_bin_operator = util::IsOperatorToken(blk->symbol.back().head_gen);
    bool reversed = (is_bin_operator && blk->need_reversing);

    if (is_bin_operator) limit = 2;
    
    while (!blk->args.empty() && !blk->args.back().IsPlaceholder()) {
      if ((is_bin_operator) && idx >= limit) break;

      reversed ?
        arguments.emplace_back(blk->args.back()) :
        arguments.emplace_front(blk->args.back());
      
      blk->args.pop_back();
      (is_bin_operator) ? idx += 1 : idx = idx;
    }

    if (!blk->args.empty()
      && blk->args.back().IsPlaceholder()
      && !util::IsOperatorToken(blk->symbol.back().head_gen)) {
      
      blk->args.pop_back();
    }

    action_base_.emplace_back(Command(blk->symbol.back(), arguments));
    blk->symbol.pop_back();
    blk->args.emplace_back(Argument("", AT_RET, kTokenTypeNull));
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
      Argument(blk->next.first,AT_NORMAL,blk->next.second)
    };

    action_base_.emplace_back(Command(Request(token), arguments));
    blk->domain = blk->args.back();
    blk->args.pop_back();
  }

  void Analyzer::LeftBracket(AnalyzerWorkBlock *blk) {
    if (blk->define_line) return;
    if (blk->last.second != TokenType::kTokenTypeGeneric) {
      blk->symbol.emplace_back(Request(kTokenNop));
    }
    
    blk->symbol.push_back(Request(blk->current.first));
    blk->args.emplace_back(Argument());
  }

  bool Analyzer::RightBracket(AnalyzerWorkBlock *blk) {
    bool result = CleanupStack(blk);
    auto &symbol = blk->symbol;

    if (result) {
      if (blk->need_reversing) blk->need_reversing = false;

      if (compare(symbol.back().head_reg, { "(","[","{" })) {
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
      Argument("__at", AT_NORMAL, kTokenTypeGeneric)
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
    bool function = false;
    bool result = true;
    GenericToken token = util::GetGenericToken(blk->current.first);

    if (blk->define_line) {
      blk->args.emplace_back(Argument(blk->current.first, AT_NORMAL, kTokenTypeGeneric));
    }
    else {
      if (blk->next.first == "=") {
        blk->args.emplace_back(Argument(blk->current.first, AT_NORMAL, kTokenTypeGeneric));
      }
      else if (blk->next.first == "(") {
        if (token != kTokenNull) {
          Request request(token);
          blk->symbol.emplace_back(request);
        }
        else {
          Request request(blk->current.first);
          if (blk->last.first == ".") {
            request.domain = blk->domain;
          }
          else {
            request.domain.type = AT_HOLDER;
          }
          blk->symbol.emplace_back(request);
        }
      }
      else {
        if (compare(blk->current.first, 
          { kStrEnd,kStrElse,kStrContinue,kStrBreak })) {
          Request request(token);
          blk->symbol.emplace_back(request);
        }
        else if (blk->current.first == kStrFn) {
          blk->define_line = true;
          blk->symbol.emplace_back(Request(kTokenFn));
          blk->symbol.emplace_back(Request("(", true));
          blk->args.emplace_back(Argument());
        }
        else {
          if (token != kTokenNull) {
            result = false;
            error_string_ = "Generic token can't be a object.";
          }
          else {
            Argument arg(blk->current.first, AT_OBJECT, kTokenTypeGeneric);
            if (blk->domain.type != AT_HOLDER) {
              arg.domain.data = blk->domain.data;
              arg.domain.type = blk->domain.type;
            }
            blk->domain = Argument();

            if (blk->insert_between_object) {
              blk->args.emplace(blk->args.begin() + blk->next_insert_index, arg);
            }
            else {
              blk->args.emplace_back(arg);
            }
          }
        }
      }
    }

    if (function && blk->next.first != "(" && 
      !compare(blk->current.first, { kStrElse,kStrEnd })) {
      health_ = false;
      result = false;
      error_string_ = "Left bracket after function is missing";
    }

    if (blk->define_line && blk->last.first == kStrFn && blk->next.first != "(") {
      health_ = false;
      result = false;
      error_string_ = "Wrong definition pattern";
    }

    return result;
  }

  void Analyzer::OtherToken(AnalyzerWorkBlock *blk) {
    if (blk->insert_between_object) {
      blk->args.emplace(blk->args.begin() + blk->next_insert_index,
        Argument(blk->current.first, AT_NORMAL, blk->current.second));
      blk->insert_between_object = false;
    }
    else {
      blk->args.emplace_back(
        Argument(blk->current.first, AT_NORMAL, blk->current.second));
    }
  }

  void Analyzer::OtherSymbol(AnalyzerWorkBlock *blk) {
    GenericToken token = util::GetGenericToken(blk->current.first);
    int currentPriority = util::GetTokenPriority(token);
    Request request(token);
    request.priority = currentPriority;

    if (blk->symbol.empty()) {
      blk->symbol.push_back(request);
    }
    else if (currentPriority < blk->symbol.back().priority
      && util::IsOperatorToken(blk->symbol.back().head_gen)) {
      auto j = blk->symbol.size() - 1;
      auto k = blk->args.size();

      while (!compare(blk->symbol[j].head_reg, { "(","[","{" })
        && (currentPriority < blk->symbol[j].priority)) {

        k == blk->args.size() ? k -= 2 : k -= 1;
        --j;
      }

      blk->symbol.insert(blk->symbol.begin() + j + 1, request);
      blk->next_insert_index = k;
      blk->insert_between_object = true;
    }
    else {
      blk->symbol.push_back(request);
    }

  }

  void Analyzer::FinalProcessing(AnalyzerWorkBlock *blk) {
    bool checked = false;
    while (!blk->symbol.empty()) {
      if (blk->symbol.back().head_reg == "(") {
        error_string_ = "Right bracket is missing";
        health_ = false;
        break;
      }

      auto firstEnum = blk->symbol.back().head_gen;
      if (blk->symbol.size() > 1 && util::IsOperatorToken(firstEnum)) {
        if (util::IsOperatorToken(blk->symbol[blk->symbol.size() - 2].head_gen)) {
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

      if (!util::IsOperatorToken(blk->symbol.back().head_gen)) {
        blk->need_reversing = false;
      }

      if (!InstructionFilling(blk)) break;
    }
  }

  bool Analyzer::CleanupStack(AnalyzerWorkBlock *blk) {
    string top_token = blk->symbol.empty()? "(" :
      blk->symbol.back().head_reg;
    bool checked = false;
    bool result = true;

    while (!blk->symbol.empty() && !compare(top_token, { "{","[","(" })
      && blk->symbol.back().head_gen != kTokenBind) {

      auto first_token = blk->symbol.back().head_gen;

      if (util::IsOperatorToken(first_token)) {
        if (util::IsOperatorToken(blk->symbol[blk->symbol.size() - 2].head_gen)) {
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
      top_token = blk->symbol.back().head_reg;
    }

    return result;
  }

  Message Analyzer::Parser() {
    auto state = true;
    const auto size = tokens_.size();
    Message result;

    AnalyzerWorkBlock *blk = new AnalyzerWorkBlock();
    blk->next_insert_index = 0;
    blk->insert_between_object = false;
    blk->need_reversing = false;
    blk->define_line = false;
    blk->domain = Argument();

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
      if (token_type == TokenType::kTokenTypeSymbol) {
        BasicToken value = GetBasicToken(blk->current.first);
        switch (value) {
        case kBasicTokenAssign: EqualMark(blk); break;
        case kBasicTokenComma: state = CleanupStack(blk); break;
        case kBasicTokenLeftSqrBracket: state = LeftSqrBracket(blk); break;
        case kBasicTokenDot:             Dot(blk); break;
        case kBasicTokenLeftBracket:    LeftBracket(blk); break;
        case kBasicTokenRightSqrBracket:state = RightBracket(blk); break;
        case kBasicTokenRightBracket:   state = RightBracket(blk); break;
        case kBasicTokenLeftCurBracket: state = LeftCurBracket(blk); break;
        case kBasicTokenRightCurBracket:state = RightBracket(blk); break;
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
    blk->next = Token();
    blk->last = Token();
    blk->current = Token();
    delete blk;
    return result;
  }

  Message Analyzer::Make(string target, size_t index) {
    vector<string> spilted_string = Scanning(target);
    Message msg = Tokenizer(spilted_string);
    this->index_ = index;
    if (msg.GetCode() >= kCodeSuccess) {
      msg = Parser();
    }
    return msg;
  }
}