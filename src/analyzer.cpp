#include "analyzer.h"

namespace kagami {
  BasicTokenEnum GetBasicToken(string src) {
    if (src == "=")   return TOKEN_EQUAL;
    if (src == ",")   return TOKEN_COMMA;
    if (src == "[")   return TOKEN_LEFT_SQRBRACKET;
    if (src == ".")   return TOKEN_DOT;
    if (src == ":")   return TOKEN_COLON;
    if (src == "(")   return TOKEN_LEFT_BRACKET;
    if (src == "]")   return TOKEN_RIGHT_SQRBRACKET;
    if (src == ")")   return TOKEN_RIGHT_BRACKET;
    if (src == "++")  return TOKEN_SELFOP;
    if (src == "--")  return TOKEN_SELFOP;
    if (src == "{")   return TOKEN_LEFT_CURBRACKET;
    if (src == "}")   return TOKEN_RIGHT_CURBRACKET;
    return TOKEN_OTHERS;
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
        if (!string_processing && util::GetTokenType(current_string) == T_BLANK) {
          current_string.clear();
        }

        string_processing ?
          delay_suspending = true :
          string_processing = true, delay_switching = true;
      }

      if (!string_processing || delay_switching) {
        temp = current_string;
        temp.append(1, current);

        if (util::GetTokenType(temp) == T_NUL) {
          auto type = util::GetTokenType(current_string);
          switch (type) {
          case T_BLANK:
            current_string.clear();
            current_string.append(1, current);
            break;
          case T_INTEGER:
            if (current == '.' && isdigit(next) != 0) {
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
          current_string = temp;
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

    if (util::GetTokenType(current_string) != T_BLANK) {
      output.emplace_back(current_string);
    }

    return output;
  }

  Message Analyzer::Tokenizer(vector<string> target) {
    bool negative_flag = false;
    vector<string> output;
    stack<string> bracket_stack;
    Token current = Token("", T_NUL),
      next = Token("", T_NUL),
      last = Token("", T_NUL);
    Message msg;
    
    tokens_.clear();
    health_ = true;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = Token(target[idx], util::GetTokenType(target[idx]));
      (idx < target.size() - 1) ?
        next = Token(target[idx + 1], util::GetTokenType(target[idx + 1])) :
        next = Token("", T_NUL);

      if (current.second == T_NUL) {
        msg = Message(kStrFatalError, kCodeBadExpression, "Unknown token - " + current.first + ".");
        break;
      }

      if (current.first == "+" || current.first == "-") {
        if (last.second == T_SYMBOL && (next.second == T_INTEGER || next.second == T_FLOAT)) {
          negative_flag = true;
        }
      }

      if (current.first == "(" || current.first == "[" || current.first == "{") {
        bracket_stack.push(current.first);
      }

      if (current.first == ")" || current.first == "]" || current.first == "}") {
        if (!bracket_stack.empty() && bracket_stack.top() != kBracketPairs.at(current.first)) {
          msg = Message(kStrFatalError, kCodeBadExpression, "Left bracket is missing.");
          break;
        }
        else {
          bracket_stack.pop();
        }
      }

      if (current.first == ",") {
        if (last.second == T_SYMBOL &&
          last.first != "]" &&
          last.first != ")" &&
          last.first != "}" &&
          last.first != "'" &&
          last.first != "++" &&
          last.first != "--") {
          msg = Message(kStrFatalError, kCodeBadExpression, "Illegal comma location.");
          break;
        }
      }

      if ((current.first == "+" || current.first == "-") && !tokens_.empty()) {
        if (tokens_.back().first == current.first) {
          tokens_.back().first.append(current.first);
        }
        else {
          tokens_.emplace_back(current);
        }
      }
      else if (negative_flag && (last.first == "+" || last.first == "-")) {
        Token res = Token(last.first + current.first, current.second);
        tokens_.back() = res;
      }
      else {
        tokens_.emplace_back(current);
      }

      last = current;
    }

    return msg;
  }

  void Analyzer::Reversing(AnalyzerWorkBlock *blk) {
    deque<Entry> *temp_symbol = new deque<Entry>();
    deque<Argument> *temp_arg = new deque<Argument>();

    while (!blk->symbol.empty()
      && blk->symbol.back().GetPriority()
      == blk->symbol[blk->symbol.size() - 2].GetPriority()) {
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
    size_t idx = 0;
    Entry &ent = blk->symbol.back();

    const size_t size = ent.GetParmSize();
    auto args = ent.GetArguments();
    auto mode = ent.GetArgumentMode();
    auto token = ent.GetTokenEnum();
    bool reversed = entry::IsOperatorToken(ent.GetTokenEnum()) && blk->need_reversing;

    idx = size;
    bool doNotUseIdx = (ent.NeedRecheck() || mode == kCodeAutoSize);

    while (!blk->args.empty() && !blk->args.back().IsPlaceholder()) {
      if (!doNotUseIdx && idx == 0) break;
      if (reversed) arguments.emplace_back(blk->args.back());
      else arguments.emplace_front(blk->args.back());
      blk->args.pop_back();
      if (!doNotUseIdx) idx--;
    }

    if (!blk->args.empty()
      && blk->args.back().IsPlaceholder()
      && !entry::IsOperatorToken(token)) {
      blk->args.pop_back();
    }
      
    auto flag = ent.GetFlag();

    if (flag == kFlagMethod || ent.IsMethod()) {
      arguments.emplace_back(Argument(blk->args.back().data, AT_OBJECT, T_GENERIC));
      blk->args.pop_back();
    }

    action_base_.emplace_back(Instruction(ent, arguments));
    blk->symbol.pop_back();
    blk->args.emplace_back(Argument("", AT_RET, T_NUL));
    return health_;
  }

  void Analyzer::EqualMark(AnalyzerWorkBlock *blk) {
    if (!blk->args.empty()) {
      blk->symbol.push_back(entry::Order(kStrBind));
    }
  }

  void Analyzer::Dot(AnalyzerWorkBlock *blk) {
    auto ent = entry::Order(kStrTypeAssert);
    deque<Argument> arguments = {
      blk->args.back(),
      Argument(blk->next.first,AT_NORMAL,blk->next.second)
    };
    action_base_.emplace_back(Instruction(ent, arguments));
  }

  void Analyzer::LeftBracket(AnalyzerWorkBlock *blk) {
    if (blk->define_line) return;
    if (blk->last.second != TokenTypeEnum::T_GENERIC) {
      auto ent = entry::Order(kStrNop);
      blk->symbol.emplace_back(ent);
    }
    blk->symbol.emplace_back(blk->current.first);
    blk->args.emplace_back(Argument());
  }

  bool Analyzer::RightBracket(AnalyzerWorkBlock *blk) {
    bool result = true;
    bool checked = false;

    string top_id = blk->symbol.back().GetId();
    while (!blk->symbol.empty()
      && top_id != "{" && top_id != "[" && top_id != "("
      && blk->symbol.back().GetTokenEnum() != GT_BIND) {

      auto firstEnum = blk->symbol.back().GetTokenEnum();
      if (entry::IsOperatorToken(firstEnum)) {
        if (entry::IsOperatorToken(
          blk->symbol[blk->symbol.size() - 2].GetTokenEnum())) {
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
      top_id = blk->symbol.back().GetId();
    }
    if (result) {
      if (blk->need_reversing) blk->need_reversing = false;
      if (blk->symbol.back().GetId() == "(" || blk->symbol.back().GetId() == "[") blk->symbol.pop_back();
      result = InstructionFilling(blk);
    }
    return result;
  }

  bool Analyzer::LeftSqrBracket(AnalyzerWorkBlock *blk) {
    bool result = true;
    auto ent = entry::Order(kStrTypeAssert);
    deque<Argument> arguments = {
      blk->args.back(),
      Argument("__at",AT_NORMAL,T_GENERIC)
    };
    action_base_.emplace_back(Instruction(ent, arguments));

    ent.SetRecheckInfo("__at", true);
    blk->symbol.emplace_back(ent);
    blk->symbol.emplace_back(blk->current.first);
    blk->args.emplace_back(Argument());

    return result;
  }

  bool Analyzer::SelfOperator(AnalyzerWorkBlock *blk) {
    bool result = true;
    if (blk->last.second == T_GENERIC) {
      if (blk->current.first == "++") {
        blk->symbol.push_back(entry::Order(kStrRightSelfInc));
      }
      else if (blk->current.first == "--") {
        blk->symbol.push_back(entry::Order(kStrRightSelfDec));
      }
    }
    else if (blk->last.second != T_GENERIC && blk->next.second == T_GENERIC) {
      if (blk->current.first == "++") {
        blk->symbol.push_back(entry::Order(kStrLeftSelfInc));
      }
      else if (blk->current.first == "--") {
        blk->symbol.push_back(entry::Order(kStrLeftSelfDec));
      }
    }
    else {
      error_string_ = "Unknown self operation";
      result = false;
    }
    return result;
  }

  bool Analyzer::LeftCurBracket(AnalyzerWorkBlock *blk) {
    bool result;
    if (blk->last.second == TokenTypeEnum::T_SYMBOL) {
      auto ent = entry::Order(kStrArray);
      blk->symbol.emplace_back(ent);
      blk->symbol.emplace_back(blk->current.first);
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

    if (blk->define_line) {
      blk->args.emplace_back(Argument(blk->current.first, AT_NORMAL, T_GENERIC));
    }
    else {
      if (blk->next.first == "=") {
        blk->args.emplace_back(Argument(blk->current.first, AT_NORMAL, T_GENERIC));
      }
      else if (blk->next.first == "(") {
        auto ent = entry::Order(blk->current.first);
        bool needDomain = (blk->last.first == ".");
        if (needDomain) {
          ent.SetRecheckInfo(blk->current.first, true);
        }
        else {
          if (!ent.Good()) {
            ent.SetRecheckInfo(blk->current.first, false);
          }
        }
        blk->symbol.push_back(ent);
      }
      else {
        if (blk->current.first == kStrEnd 
          || blk->current.first == kStrElse
          || blk->current.first == kStrContinue
          || blk->current.first == kStrBreak) {
          auto ent = entry::Order(blk->current.first);
          blk->symbol.push_back(ent);
        }
        else if (blk->current.first == kStrDef) {
          auto ent = entry::Order(blk->current.first);
          blk->define_line = true;
          blk->symbol.emplace_back(ent);
          blk->symbol.emplace_back("(");
          blk->args.emplace_back(Argument());
        }
        else {
          blk->args.emplace_back(Argument(blk->current.first, AT_OBJECT, T_GENERIC));
        }
      }
    }

    if (function && blk->next.first != "("
      && blk->current.first != kStrElse 
      && blk->current.first != kStrEnd
      && blk->current.first != kStrFinally) {
      health_ = false;
      result = false;
      error_string_ = "Left bracket after function is missing";
    }

    if (blk->define_line && blk->last.first == kStrDef && blk->next.first != "(") {
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
    Entry ent = entry::Order(blk->current.first);
    int currentPriority = ent.GetPriority();
    if (blk->symbol.empty()) {
      blk->symbol.push_back(ent);
    }
    else if (currentPriority < blk->symbol.back().GetPriority()) {
      auto j = blk->symbol.size() - 1;
      auto k = blk->args.size();
      while (kBracketPairs.find(blk->symbol[j].GetId()) == kBracketPairs.end()
        && currentPriority < blk->symbol[j].GetPriority()) {
        k == blk->args.size() ? k -= 2 : k -= 1;
        --j;
      }
      blk->symbol.insert(blk->symbol.begin() + j + 1, ent);
      blk->next_insert_index = k;
      blk->insert_between_object = true;
    }
    else {
      blk->symbol.push_back(ent);
    }
  }

  void Analyzer::FinalProcessing(AnalyzerWorkBlock *blk) {
    bool checked = false;
    while (!blk->symbol.empty()) {
      if (blk->symbol.back().GetId() == "(") {
        error_string_ = "Right bracket is missing";
        health_ = false;
        break;
      }
      auto firstEnum = blk->symbol.back().GetTokenEnum();
      if (blk->symbol.size() > 1 && entry::IsOperatorToken(firstEnum)) {
        if (entry::IsOperatorToken(blk->symbol[blk->symbol.size() - 2].GetTokenEnum())) {
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
      if (!entry::IsOperatorToken(blk->symbol.back().GetTokenEnum())) {
        blk->need_reversing = false;
      }
      if (!InstructionFilling(blk)) break;
    }
  }

  Message Analyzer::Parser() {
    using namespace entry;
    auto state = true;
    const auto size = tokens_.size();
    Message result;

    AnalyzerWorkBlock *blk = new AnalyzerWorkBlock();
    blk->next_insert_index = 0;
    blk->insert_between_object = false;
    blk->need_reversing = false;
    blk->define_line = false;

    for (size_t i = 0; i < size; ++i) {
      if (!health_) break;
      if (!state)  break;

      blk->current = tokens_[i];
      i < size - 1 ?
        blk->next = tokens_[i + 1] :
        blk->next = Token(kStrNull, TokenTypeEnum::T_NUL);

      auto token_type = blk->current.second;
      if (token_type == TokenTypeEnum::T_SYMBOL) {
        BasicTokenEnum value = GetBasicToken(blk->current.first);
        switch (value) {
        case TOKEN_EQUAL: EqualMark(blk); break;
        case TOKEN_COMMA: break;
        case TOKEN_COLON: break;
        case TOKEN_LEFT_SQRBRACKET: state = LeftSqrBracket(blk); break;
        case TOKEN_DOT:             Dot(blk); break;
        case TOKEN_LEFT_BRACKET:    LeftBracket(blk); break;
        case TOKEN_RIGHT_SQRBRACKET:state = RightBracket(blk); break;
        case TOKEN_RIGHT_BRACKET:   state = RightBracket(blk); break;
        case TOKEN_SELFOP:          state = SelfOperator(blk); break;
        case TOKEN_LEFT_CURBRACKET: state = LeftCurBracket(blk); break;
        case TOKEN_RIGHT_CURBRACKET:state = RightBracket(blk); break;
        case TOKEN_OTHERS:          OtherSymbol(blk); break;
        default:break;
        }
      }
      else if (token_type == TokenTypeEnum::T_GENERIC) {
        state = FunctionAndObject(blk);
      }
      else if (token_type == TokenTypeEnum::T_NUL) {
        result = Message(kStrFatalError, kCodeIllegalParm, "Illegal token.");
        state = false;
      }
      else OtherToken(blk);
      blk->last = blk->current;
    }

    if (state) {
      FinalProcessing(blk);
    }
    if (!state || !health_) {
      result = Message(kStrFatalError, kCodeBadExpression, error_string_);
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

  void Analyzer::Clear() {
    tokens_.clear();
    tokens_.shrink_to_fit();
    action_base_.clear();
    action_base_.shrink_to_fit();
    health_ = false;
    error_string_.clear();
    index_ = 0;
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