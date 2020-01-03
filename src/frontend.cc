#include "frontend.h"

#define ERROR_MSG(_Msg) Message(_Msg, kStateError)

namespace kagami {
  inline bool IsBranchKeyword(Keyword keyword) {
    return keyword == kKeywordElif || keyword == kKeywordElse || keyword == kKeywordWhen;
  }

  inline bool IsReservedKeyword(Keyword keyword) {
    return keyword == kKeywordIf ||
      keyword == kKeywordElif ||
      keyword == kKeywordWhile ||
      keyword == kKeywordReturn ||
      keyword == kKeywordWhen ||
      keyword == kKeywordCase;
  }

  inline bool IsNestRoot(Keyword keyword) {
    return keyword == kKeywordIf ||
      keyword == kKeywordWhile ||
      keyword == kKeywordFn ||
      keyword == kKeywordCase ||
      keyword == kKeywordFor;
  }

  inline bool IsSingleKeyword(Keyword keyword) {
    return keyword == kKeywordEnd ||
      keyword == kKeywordElse ||
      keyword == kKeywordContinue ||
      keyword == kKeywordBreak;
  }

  string GetLeftBracket(string rhs) {
    if (rhs == ")") return "(";
    if (rhs == "]") return "[";
    if (rhs == "}") return "{";
    return string();
  }

  string IndentationAndCommentProc(string target) {
    if (target == "") return "";
    string data;
    char current = 0, last = 0;
    size_t head = 0, tail = 0;
    bool exempt_blank_char = true;
    bool string_processing = false;
    auto toString = [](char t) ->string {return string().append(1, t); };

    for (size_t count = 0; count < target.size(); ++count) {
      current = target[count];
      auto type = util::GetStringType(toString(current));
      if (type != StringType::kStringTypeBlank && exempt_blank_char) {
        head = count;
        exempt_blank_char = false;
      }
      if (current == '\'' && last != '\\') {
        string_processing = !string_processing;
      }
      if (!string_processing && current == '#') {
        tail = count;
        break;
      }
      last = target[count];
    }
    if (tail > head) data = target.substr(head, tail - head);
    else data = target.substr(head, target.size() - head);
    if (data.front() == '#') return "";

    while (!data.empty() &&
      util::GetStringType(toString(data.back())) == kStringTypeBlank) {
      data.pop_back();
    }
    return data;
  }

  void LexicalFactory::Scan(deque<string> &output, string target) {
    string current_token, temp;
    bool inside_string = false;
    bool leave_string = false;
    bool enter_string = false;
    bool escape_flag = false;
    bool not_escape_char = false;
    char current = 0, next = 0, last = 0;

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
        if (!inside_string && util::GetStringType(current_token) == kStringTypeBlank) {
          current_token.clear();
        }

        inside_string ?
          leave_string = true :
          inside_string = true, enter_string = true;
      }

      if (!inside_string || enter_string) {
        temp = current_token;
        temp.append(1, current);

        auto type = util::GetStringType(temp);
        if (type == kStringTypeNull) {
          auto type = util::GetStringType(current_token);
          switch (type) {
          case kStringTypeBlank:
            current_token.clear();
            current_token.append(1, current);
            break;
          case kStringTypeInt:
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
          if (type == kStringTypeInt && (temp[0] == '+' || temp[0] == '-')) {
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

    if (util::GetStringType(current_token) != kStringTypeBlank) {
      output.emplace_back(current_token);
    }

  }

  bool LexicalFactory::Feed(CombinedCodeline &src) {
    bool good = true;
    bool negative_flag = false;
    stack<string> bracket_stack;
    deque<string> target;
    Token current = INVALID_TOKEN;
    Token next = INVALID_TOKEN;
    Token last = INVALID_TOKEN;

    Scan(target, src.second);

    dest_->emplace_back(CombinedToken(src.first, deque<Token>()));

    auto *tokens = &dest_->back().second;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = Token(target[idx], util::GetStringType(target[idx]));
      next = (idx < target.size() - 1) ?
        Token(target[idx + 1], util::GetStringType(target[idx + 1])) :
        INVALID_TOKEN;

      if (current.first == ";") {
        if (!bracket_stack.empty()) {
          trace::AddEvent("Invalid end of statment at line " +
            to_string(src.first), kStateError);
          good = false;
          break;
        }

        if (idx == target.size() - 1) {
          trace::AddEvent("Unnecessary semicolon at line " +
            to_string(src.first), kStateWarning);
        }
        else {
          dest_->emplace_back(CombinedToken(src.first, deque<Token>()));
          tokens = &dest_->back().second;
        }

        continue;
      }

      if (current.second == kStringTypeNull) {
        trace::AddEvent("Unknown token - " + current.first +
          " at line " + to_string(src.first), kStateError);
        good = false;
        break;
      }

      if (compare(current.first, "+", "-") && !compare(last.first, ")", "]", "}")) {
        if (compare(last.second, kStringTypeSymbol, kStringTypeNull) &&
          compare(next.second, kStringTypeInt, kStringTypeFloat)) {
          negative_flag = true;
          tokens->push_back(current);
          last = current;
          continue;
        }
      }

      if (compare(current.first, "(", "[", "{")) {
        bracket_stack.push(current.first);
      }

      if (compare(current.first, ")", "]", "}")) {
        if (bracket_stack.empty()) {
          trace::AddEvent("Left bracket is missing - " + current.first +
            " at line " + to_string(src.first), kStateError);
          good = false;
          break;
        }

        if (GetLeftBracket(current.first) != bracket_stack.top()) {
          trace::AddEvent("Left bracket is missing - " + current.first +
            " at line " + to_string(src.first), kStateError);
          good = false;
          break;
        }

        bracket_stack.pop();
      }

      if (current.first == ",") {
        if (last.second == kStringTypeSymbol &&
          !compare(last.first, "]", ")", "}", "'")) {
          trace::AddEvent("Invalid comma at line " + to_string(src.first), kStateError);
          good = false;
          break;
        }
      }

      if (compare(last.first, "+", "-") && negative_flag) {
        Token combined(last.first + current.first, current.second);
        tokens->back() = combined;
        negative_flag = false;
        last = combined;
        continue;
      }


      tokens->push_back(current);
      last = current;
    }

    return good;
  }

  void ParserFrame::Eat() {
    if (idx < tokens.size()) {
      last = current;
      current = tokens[idx];
      next = idx + 1 < tokens.size() ? tokens[idx + 1] : INVALID_TOKEN;
      next_2 = idx + 2 < tokens.size() ? tokens[idx + 2] : INVALID_TOKEN;
      idx += 1;
    }

    if (idx == tokens.size()) {
      eol = true;
    }
  }

  void LineParser::ProduceVMCode() {
    deque<Argument> arguments;
    size_t idx = 0, limit = 0;

    if (frame_->symbol.back().IsPlaceholder()) frame_->symbol.pop_back();

    bool is_bin_operator = 
      util::IsBinaryOperator(frame_->symbol.back().GetKeywordValue());
    bool is_mono_operator = 
      util::IsMonoOperator(frame_->symbol.back().GetKeywordValue());

    if (is_bin_operator) limit = 2;
    if (is_mono_operator) limit = 1;
    
    while (!frame_->args.empty() && !frame_->args.back().IsPlaceholder()) {
      if ((is_bin_operator || is_mono_operator) && idx >= limit) break;

      arguments.emplace_front(frame_->args.back());
      
      frame_->args.pop_back();
      (is_bin_operator || is_mono_operator) ? idx += 1 : idx = idx;
    }

    if (!frame_->args.empty()
      && frame_->args.back().IsPlaceholder()
      && !is_bin_operator && !is_mono_operator) {
      
      frame_->args.pop_back();
    }

    action_base_.emplace_back(Command(frame_->symbol.back(), arguments));
    frame_->symbol.pop_back();
    frame_->args.emplace_back(Argument("", kArgumentReturnStack, kStringTypeNull));
    if (frame_->symbol.empty() && (frame_->next.first == "," 
      || frame_->next.second == kStringTypeNull)) {
      action_base_.back().first.option.void_call = true;
    }
  }

  bool LineParser::CleanupStack() {
    bool result = true;

    while (!frame_->symbol.empty() && !frame_->symbol.back().IsPlaceholder()) {
      ProduceVMCode();
    }

    return result;
  }

  void LineParser::BindExpr() {
    if (!frame_->args.empty()) {
      Request request(kKeywordBind);
      request.option.local_object = frame_->local_object;
      frame_->local_object = false;
      frame_->symbol.emplace_back(request);
    }
  }

  void LineParser::DeliveringExpr() {
    if (!frame_->args.empty()) {
      Request request(kKeywordDelivering);
      request.option.local_object = frame_->local_object;
      frame_->local_object = false;
      frame_->symbol.emplace_back(request);
    }
  }

  void LineParser::DotExpr() {
    frame_->domain = frame_->args.back();
    frame_->args.pop_back();
  }

  void LineParser::UnaryExpr() {
    auto token = util::GetKeywordCode(frame_->current.first);
    frame_->symbol.emplace_back(Request(token));
  }

  void LineParser::FuncInvokingExpr() {
    if (frame_->last.second != kStringTypeIdentifier) {
      frame_->symbol.emplace_back(Request(kKeywordExpList));
    }

    if (compare(util::GetKeywordCode(frame_->last.first),
      kKeywordIf, kKeywordElif, kKeywordWhile,
      kKeywordCase, kKeywordWhen, kKeywordReturn)) {
      frame_->symbol.emplace_back(Request(kKeywordExpList));
    }
    
    frame_->symbol.push_back(Request());
    frame_->args.emplace_back(Argument());
  }

  bool LineParser::IndexExpr() {
    Request request("__at", frame_->args.back());
    frame_->symbol.emplace_back(request);
    frame_->symbol.emplace_back(Request());
    frame_->args.pop_back();
    frame_->args.emplace_back(Argument());

    return true;
  }

  bool LineParser::ArrayExpr() {
    bool result = true;
    if (frame_->last.second == StringType::kStringTypeSymbol) {
      frame_->symbol.emplace_back(Request(kKeywordInitialArray));
      frame_->symbol.emplace_back(Request());
      frame_->args.emplace_back(Argument());
      result = true;
    }
    else {
      result = false;
      error_string_ = "Illegal brace location.";
    }
    return result;
  }

  void LineParser::BinaryExpr() {
    auto token = util::GetKeywordCode(frame_->current.first);
    int current_priority = util::GetTokenPriority(token);
    Request request(token);

    if (!frame_->symbol.empty()) {
      bool is_operator =
        util::IsBinaryOperator(frame_->symbol.back().GetKeywordValue());
      int stack_top_priority =
        util::GetTokenPriority(frame_->symbol.back().GetKeywordValue());

      auto checking = [&stack_top_priority, &current_priority]()->bool {
        return (stack_top_priority >= current_priority);
      };

      while (!frame_->symbol.empty() && is_operator && checking()) {
        ProduceVMCode();
        is_operator =
          (!frame_->symbol.empty() 
          && util::IsBinaryOperator(frame_->symbol.back().GetKeywordValue()));
        stack_top_priority = frame_->symbol.empty() ? 5 :
          util::GetTokenPriority(frame_->symbol.back().GetKeywordValue());
      }
    }

    frame_->symbol.emplace_back(request);
  }

  bool LineParser::FnExpr() {
    //TODO:Preprecessing-time argument type checking
    if (frame_->last.second != kStringTypeNull) {
      error_string_ = "Invalid function definition";
      return false;
    }

    if (tokens_.size() < 4) {
      error_string_ = "Invalid function definition";
      return false;
    }

    if (frame_->next_2.first != "(") {
      error_string_ = "Invalid function definition";
      return false;
    }

    bool left_paren = false;
    bool inside_params = false;
    bool optional = false, optional_declared = false;
    bool variable = false, variable_declared = false;
    bool good = true;

    frame_->symbol.emplace_back(Request(kKeywordFn));
    frame_->symbol.emplace_back(Request());
    frame_->args.emplace_back(Argument());

    auto invalid_param_pattern = [&]()->bool {
      return frame_->next.first == "," ||
        frame_->next.first == kStrOptional ||
        frame_->next.first == kStrVariable;
    };

    //Function identifier
    frame_->Eat();
    if (auto &str = frame_->current.first; (str == kStrOptional || str == kStrVariable)) {
      error_string_ = "Invalid function identifier";
      return false;
    }

    frame_->args.emplace_back(
      Argument(frame_->current.first, kArgumentNormal, kStringTypeIdentifier));

    //Parameter segment
    while (!frame_->eol) {
      frame_->Eat();

      if (frame_->current.first == "(") {
        if (left_paren) {
          error_string_ = "Invalid symbol in function definition";
          good = false;
          break;
        }
        else {
          left_paren = true;
          inside_params = true;
          continue;
        }
      }
      else if (frame_->current.first == ")") {
        inside_params = false;
        ProduceVMCode();
        break;
      }
      else if (frame_->current.first == kStrOptional) {
        if (invalid_param_pattern()) {
          error_string_ = "Invalid function parameter declaration.";
          good = false;
          break;
        }

        if (variable_declared) {
          error_string_ = "Cannot declare optional/variable parameters at same time";
          good = false;
          break;
        }

        optional = true;
        if (!optional_declared) optional_declared = true;
      }
      else if (frame_->current.first == kStrVariable) {
        if (invalid_param_pattern()) {
          error_string_ = "Invalid function parameter declaration.";
          good = false;
          break;
        }

        if (variable_declared) {
          error_string_ = "Cannot declare multiple variable parameters at same time";
          good = false;
          break;
        }

        if (optional_declared) {
          error_string_ = "Cannot declare optional/variable parameters at same time";
          good = false;
          break;
        }

        variable = true;
        if (!variable_declared) variable_declared = true;
      }
      else if (frame_->current.first == ",") continue;
      else if (frame_->current.second != kStringTypeIdentifier) {
        error_string_ = "Invalid value in function definition";
        good = false;
        break;
      }
      else {
        Argument arg(frame_->current.first, kArgumentNormal, kStringTypeIdentifier);
        if (optional) {
          arg.option.optional_param = true;
          optional = false;
        }

        if (variable) {
          arg.option.variable_param = true;
          variable = false;
        }

        frame_->args.emplace_back(arg);
      }
    }

    return good;
  }

  bool LineParser::ForEachExpr() {
    if (frame_->last.second != kStringTypeNull) {
      error_string_ = "Invalid for-each expression";
      return false;
    }

    if (tokens_.size() < 4) {
      error_string_ = "Invalid for-each expression";
      return false;
    }

    bool good = true;

    frame_->symbol.emplace_back(Request(kKeywordFor));
    frame_->symbol.emplace_back(Request());
    frame_->args.emplace_back(Argument());

    if (frame_->Eat(); util::GetStringType(frame_->current.first) != kStringTypeIdentifier) {
      error_string_ = "Invalid identifier argument in for-each expression";
      return false;
    }

    frame_->args.emplace_back(Argument(
      frame_->current.first, kArgumentNormal, kStringTypeIdentifier));

    
    if (frame_->Eat(); util::GetTerminatorCode(frame_->current.first) != kTerminatorIn) {
      error_string_ = "Invalid for-each expression";
      return false;
    }

    return good;
  }

  bool LineParser::OtherExpressions() {
    Keyword token = util::GetKeywordCode(frame_->current.first);

    if (IsSingleKeyword(token)) {
      if (frame_->next.second != kStringTypeNull) {
        error_string_ = "Invalid syntax after " + frame_->current.first;
        return false;
      }

      Request request(token);
      frame_->symbol.emplace_back(request);
      return true;
    }

    if (token == kKeywordLocal) {
      if (frame_->next_2.first != "=") {
        error_string_ = "Invalid 'local' token.";
        return false;
      }

      frame_->local_object = true;
      return true;
    }

    if (token == kKeywordExt) {
      if (frame_->next_2.first != "=") {
        error_string_ = "Invalid 'local' token.";
        return false;
      }

      frame_->ext_object = true;
      return true;
    }

    if (token != kKeywordNull) {
      if (frame_->next.first == "=" || util::IsOperator(token)) {
        error_string_ = "Trying to operate with reserved keyword";
        return false;
      }

      if (IsReservedKeyword(token)) {
        frame_->symbol.emplace_back(Request(token));
        frame_->args.emplace_back(Argument());
        return true;
      }
      else {
        if (frame_->next.first != "(") {
          error_string_ = "Invalid syntax after " + frame_->current.first;
          return false;
        }
      }

      Request request(token);
      frame_->symbol.emplace_back(request);

      return true;
    }


    if (frame_->next.first == "(") {
      Request request(frame_->current.first,
        frame_->domain.IsPlaceholder() ?
        Argument() : frame_->domain
      );
      frame_->symbol.emplace_back(request);
      frame_->domain = Argument();
      return true;
    }

    if (frame_->next.first == "=" || frame_->next.first == "<-") {
      frame_->args.emplace_back(Argument(
        frame_->current.first, kArgumentNormal, kStringTypeIdentifier));
      return true;
    }

    
    if (frame_->domain.GetType() != kArgumentNull) {
      //TODO:argument domain appending
      error_string_ = "Invalid expression";
      return false;
    }
    else {
      Argument arg(
        frame_->current.first, kArgumentObjectStack, kStringTypeIdentifier);
      frame_->args.emplace_back(arg);
    }
    
    return true;
  }

  void LineParser::LiteralValue() {
    frame_->args.emplace_back(
      Argument(frame_->current.first, kArgumentNormal, frame_->current.second));
  }


  Message LineParser::Parse() {
    auto state = true;
    const auto size = tokens_.size();
    Message result;

    frame_ = new ParserFrame(tokens_);

    while (!frame_->eol) {
      if (!state) break;
      frame_->Eat();

      if (Terminator value = util::GetTerminatorCode(frame_->current.first); 
        value != kTerminatorNull) {
        switch (value) {
        case kTerminatorAssign:
          BindExpr();
          break;
        case kTerminatorArrow:
          DeliveringExpr();
          break;
        case kTerminatorComma:
          state = CleanupStack();
          break;
        case kTerminatorDot:
          DotExpr();
          break;
        case kTerminatorLeftParen:
          FuncInvokingExpr();
          break;
        case kTerminatorLeftBracket:
          state = IndexExpr();
          break;
        case kTerminatorLeftBrace:
          state = ArrayExpr();
          break;
        case kTerminatorMonoOperator:
          UnaryExpr();
          break;
        case kTerminatorBinaryOperator:
          BinaryExpr();
          break;
        case kTerminatorFn:
          state = FnExpr();
          break;
        case kTerminatorFor:
          state = ForEachExpr();
          break;
        case kTerminatorRightSqrBracket:
        case kTerminatorRightBracket:
        case kTerminatorRightCurBracket:
          state = CleanupStack();
          if (state) ProduceVMCode();
          break;
        default:
          break;
        }
      }
      else {
        auto token_type = frame_->current.second;

        if (token_type == kStringTypeIdentifier) {
          state = OtherExpressions();
        }
        else {
          LiteralValue();
        }
      }
    }

    if (state) {
      while (!frame_->symbol.empty()) {
        ProduceVMCode();
      }
    }
    else {
      result = ERROR_MSG(error_string_).SetIndex(index_);
    }

    for (auto &unit : action_base_) {
      unit.first.idx = index_;
    }

    frame_->args.clear();
    frame_->args.shrink_to_fit();
    frame_->symbol.clear();
    frame_->symbol.shrink_to_fit();
    delete frame_;
    return result;
  }

  void LineParser::Clear() {
    tokens_.clear();
    tokens_.shrink_to_fit();
    action_base_.clear();
    action_base_.shrink_to_fit();
    error_string_.clear();
    index_ = 0;
  }

  Message LineParser::Make(CombinedToken &line) {
    index_ = line.first;
    tokens_.clear();
    tokens_ = line.second;
    return Parse().SetIndex(line.first);
  }

  bool VMCodeFactory::ReadScript(list<CombinedCodeline> &dest) {
    bool inside_comment_block = false;
    size_t idx = 1;
    string buf;
    InStream reader(path_);

    if (!reader.Good()) return false;

    while (!reader.eof()) {
      buf = reader.GetLine();

      if (buf == kStrCommentBegin) {
        inside_comment_block = true;
        idx += 1;
        continue;
      }

      if (inside_comment_block) {
        inside_comment_block = (buf == kStrCommentEnd);
        idx += 1;
        continue;
      }

      buf = IndentationAndCommentProc(buf);

      if (buf.empty()) {
        idx += 1;
        continue;
      }

      dest.push_back(CombinedCodeline(idx, buf));
      idx += 1;
    }

    return true;
  }

  bool VMCodeFactory::Start() {
    bool good = true;
    LexicalFactory lexer(tokens_);
    LineParser line_parser;
    Message msg;
    VMCode anchorage;
    StateLevel level;
    Keyword ast_root;

    if (!ReadScript(script_)) return false;

    for (auto it = script_.begin(); it != script_.end(); ++it) {
      good = lexer.Feed(*it);
      if (!good) break;
    }

    if (!good) return false;

    for (auto it = tokens_.begin(); it != tokens_.end(); ++it) {
      if (!good) break;

      msg = line_parser.Make(*it);

      level = msg.GetLevel();
      ast_root = line_parser.GetASTRoot();

      if (level == kStateError) {
        trace::AddEvent(msg);
        good = false;
        continue;
      }
      else if (level == kStateWarning) {
        trace::AddEvent(msg);
      }

      anchorage.swap(line_parser.GetOutput());
      line_parser.Clear();

      if (IsNestRoot(ast_root)) {
        if (ast_root == kKeywordIf || ast_root == kKeywordCase) {
          jump_stack_.push(JumpListFrame{ ast_root,
            dest_->size() + anchorage.size() - 1 });
        }

        if (ast_root == kKeywordWhile || ast_root == kKeywordFor) {
          cycle_escaper_.push(nest_.size() + 1);
        }

        nest_.push(dest_->size());
        nest_end_.push(dest_->size() + anchorage.size() - 1);
        nest_origin_.push(it->first);
        nest_type_.push(ast_root);
        dest_->insert(dest_->end(), anchorage.begin(), anchorage.end());
        anchorage.clear();
        continue;
      }

      if (IsBranchKeyword(ast_root)) {
        if (jump_stack_.empty()) {
          trace::AddEvent("Invalid branch keyword at line " + to_string(it->first), kStateError);
          break;
        }

        if (jump_stack_.top().nest_code == kKeywordIf) {
          if (ast_root == kKeywordElif || ast_root == kKeywordElse) {
            jump_stack_.top().jump_record.push_back(dest_->size());
          }
          else {
            trace::AddEvent("Invalid branch keyword at line " + to_string(it->first));
            break;
          }
        }
        else if (jump_stack_.top().nest_code == kKeywordCase) {
          if (ast_root == kKeywordWhen || ast_root == kKeywordElse) {
            jump_stack_.top().jump_record.push_back(dest_->size());
          }
          else {
            trace::AddEvent("Invalid branch keyword at line " + to_string(it->first), kStateError);
            break;
          }
        }
      }

      if (ast_root == kKeywordContinue || ast_root == kKeywordBreak) {
        if (cycle_escaper_.empty()) {
          trace::AddEvent("Invalid cycle escaper at line " + to_string(it->first), kStateError);
          break;
        }

        anchorage.back().first.option.escape_depth = nest_.size() - cycle_escaper_.top();
      }

      if (ast_root == kKeywordEnd) {
        if (nest_type_.empty()) {
          trace::AddEvent("Invalid 'end' token at line " + to_string(it->first), kStateError);
          good = false;
          break;
        }

        if (!cycle_escaper_.empty() && nest_.size() == cycle_escaper_.top())
          cycle_escaper_.pop();

        (*dest_)[nest_end_.top()].first.option.nest_end = dest_->size();
        anchorage.back().first.option.nest_root = nest_type_.top();
        anchorage.back().first.option.nest = nest_.top();

        if (compare(nest_type_.top(), kKeywordIf, kKeywordCase) && !jump_stack_.empty()){
          if (!jump_stack_.top().jump_record.empty()) {
            dest_->AddJumpRecord(jump_stack_.top().nest, jump_stack_.top().jump_record);
          }
          jump_stack_.pop();
        }

        nest_.pop();
        nest_end_.pop();
        nest_origin_.pop();
        nest_type_.pop();
      }

      dest_->insert(dest_->end(), anchorage.begin(), anchorage.end());
      anchorage.clear();
    }

    if (!nest_.empty()) {
      trace::AddEvent("'end' token is not found for line " + 
        to_string(nest_origin_.top()), kStateError);
      good = false;
    }

    return good;
  }
}
