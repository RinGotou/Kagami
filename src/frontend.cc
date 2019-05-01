#include "frontend.h"

#define INVALID_TOKEN Token(string(), kStringTypeNull)
#define ERROR_MSG(_Msg) Message(kCodeBadExpression, _Msg, kStateError)

namespace kagami {
  /* Disposing all indentation and comments */
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



  vector<string> LineParser::Scanning(string target) {
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

    return output;
  }

  Message LineParser::Tokenizer(vector<string> target) {
    bool negative_flag = false;
    stack<string> bracket_stack;
    Token current = INVALID_TOKEN;
    Token next = INVALID_TOKEN;
    Token last = INVALID_TOKEN;
    Message msg;
    
    tokens_.clear();

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = Token(target[idx], util::GetStringType(target[idx]));
      next = (idx < target.size() - 1) ?
        Token(target[idx + 1], util::GetStringType(target[idx + 1])) :
        INVALID_TOKEN;

      if (current.second == kStringTypeNull) {
        msg = ERROR_MSG("Unknown token - " + current.first);
        break;
      }

      if (compare_exp(current.first, "+", "-")) {
        if (compare_exp(last.second, kStringTypeSymbol, kStringTypeNull) && 
            compare_exp(next.second, kStringTypeInt, kStringTypeFloat)) {
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
        if (last.second == kStringTypeSymbol &&
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

  void LineParser::ProduceVMCode(ParserBlock *blk) {
    deque<Argument> arguments;
    size_t idx = 0, limit = 0;

    if (blk->symbol.back().IsPlaceholder()) blk->symbol.pop_back();

    bool is_bin_operator = util::IsBinaryOperator(blk->symbol.back().keyword_value);
    bool is_mono_operator = util::IsMonoOperator(blk->symbol.back().keyword_value);

    if (is_bin_operator) limit = 2;
    if (is_mono_operator) limit = 1;
    
    while (!blk->args.empty() && !blk->args.back().IsPlaceholder()) {
      if ((is_bin_operator || is_mono_operator) && idx >= limit) break;

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
    blk->args.emplace_back(Argument("", kArgumentReturnStack, kStringTypeNull));
    if (blk->symbol.empty() && (blk->next.first == "," 
      || blk->next.second == kStringTypeNull)) {
      action_base_.back().first.option.void_call = true;
    }
  }

  void LineParser::BindExpr(ParserBlock *blk) {
    if (!blk->args.empty() && blk->next.first != kStrFn) {
      Request request(kKeywordBind);
      request.option.local_object = blk->local_object;
      request.priority = util::GetTokenPriority(kKeywordBind);
      blk->local_object = false;
      blk->symbol.emplace_back(request);
    }
  }

  void LineParser::DotExpr(ParserBlock *blk) {
    blk->domain = blk->args.back();
    blk->args.pop_back();
  }

  void LineParser::UnaryExpr(ParserBlock *blk) {
    auto token = util::GetKeywordCode(blk->current.first);
    blk->symbol.emplace_back(Request(token));
  }

  void LineParser::FuncInvokingExpr(ParserBlock *blk) {
    if (blk->fn_expr) return;
    if (blk->last.second != StringType::kStringTypeIdentifier) {
      blk->symbol.emplace_back(Request(kKeywordExpList));
    }
    
    blk->symbol.push_back(Request());
    blk->args.emplace_back(Argument());
  }

  bool LineParser::IndexExpr(ParserBlock *blk) {
    bool result = true;

    deque<Argument> arguments = {
      blk->args.back(),
      Argument("__at", kArgumentNormal, kStringTypeIdentifier)
    };

    Request request("__at");
    request.domain = blk->args.back();
    blk->symbol.emplace_back(request);
    blk->symbol.emplace_back(Request());
    blk->args.pop_back();
    blk->args.emplace_back(Argument());

    return result;
  }

  bool LineParser::ArrayExpr(ParserBlock *blk) {
    bool result;
    if (blk->last.second == StringType::kStringTypeSymbol) {
      blk->symbol.emplace_back(Request(kKeywordInitialArray));
      blk->symbol.emplace_back(Request());
      blk->args.emplace_back(Argument());
      result = true;
    }
    else {
      result = false;
      error_string_ = "Illegal brace location.";
    }
    return result;
  }

  bool LineParser::FunctionAndObject(ParserBlock *blk) {
    Keyword token = util::GetKeywordCode(blk->current.first);
    auto token_type = util::GetStringType(blk->current.first);
    auto token_next = util::GetKeywordCode(blk->next.first);
    auto token_next_2 = util::GetKeywordCode(blk->next_2.first);
    bool lambda_func = false;

    if (find_in_vector(token, kSingleWordStore)) {
      if (blk->next.second != kStringTypeNull) {
        error_string_ = "Invalid syntax after " + blk->current.first;
        return false;
      }

      Request request(token);
      blk->symbol.emplace_back(request);
      return true;
    }

    if (blk->fn_expr) {
      if (token_type != kStringTypeIdentifier) {
        error_string_ = "Invalid token in function definition";
        return false;
      }

      if (blk->current.first == kStrOptional || blk->current.first == kStrVariable) {
        if (util::GetStringType(blk->next.first) != kStringTypeIdentifier) {
          error_string_ = "Invalid syntax after " + blk->current.first;
          return false;
        }
      }

      blk->args.emplace_back(Argument(blk->current.first, kArgumentNormal, kStringTypeIdentifier));
      return true;
    }

    if (token == kKeywordFn) {
      if (blk->next.first == "(" && blk->next.second == kStringTypeIdentifier) {
        lambda_func = true;
      }

      if (blk->next.first != "(" && blk->next_2.first != "(") {
        error_string_ = "Invalid syntax after fn";
        return false;
      }

      blk->fn_expr = true;
      Request request(kKeywordFn);
      //request.option.lambda_fn_obj = lambda_func;
      blk->symbol.emplace_back(request);
      blk->symbol.emplace_back(Request());
      blk->args.emplace_back(Argument());
      return true;
    }

    if (token == kKeywordFor) {
      if (token_next_2 != kKeywordIn) {
        error_string_ = "Invalid syntax after for";
        return false;
      }

      if (blk->next.second != kStringTypeIdentifier) {
        error_string_ = "Invalid unit name after for";
        return false;
      }

      blk->foreach_expr = true;
      blk->symbol.emplace_back(Request(kKeywordFor));
      blk->args.emplace_back(Argument());
      return true;
    }

    if (token == kKeywordLocal) {
      if (blk->next_2.first != "=") {
        error_string_ = "Invalid 'local' token.";
        return false;
      }

      blk->local_object = true;
      return true;
    }

    if (token == kKeywordIn) {
      if (!blk->foreach_expr) {
        error_string_ = "Invalid 'in' token";
        return false;
      }

      return true;
    }

    if (blk->foreach_expr && token_next == kKeywordIn) {
      blk->args.emplace_back(Argument(
        blk->current.first, kArgumentNormal, kStringTypeIdentifier));
      return true;
    }

    if (token != kKeywordNull) {
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
        blk->current.first, kArgumentNormal, kStringTypeIdentifier));
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
        blk->current.first, kArgumentObjectStack, kStringTypeIdentifier);
      blk->args.emplace_back(arg);
    }
    
    return true;
  }

  void LineParser::OtherToken(ParserBlock *blk) {
    blk->args.emplace_back(
      Argument(blk->current.first, kArgumentNormal, blk->current.second));
  }

  void LineParser::BinaryExpr(ParserBlock *blk) {
    auto token = util::GetKeywordCode(blk->current.first);
    int current_priority = util::GetTokenPriority(token);
    Request request(token);
    request.priority = current_priority;

    //TODO:processing for same level
    if (!blk->symbol.empty()) {
      bool stack_top_operator = util::IsBinaryOperator(blk->symbol.back().keyword_value);
      int stack_top_priority = util::GetTokenPriority(blk->symbol.back().keyword_value);

      auto checking = [&stack_top_priority, &current_priority]()->bool {
        return (stack_top_priority >= current_priority);
      };

      while (!blk->symbol.empty() && stack_top_operator && checking()) {
        ProduceVMCode(blk);
        stack_top_operator = 
          (!blk->symbol.empty() && util::IsBinaryOperator(blk->symbol.back().keyword_value));
        stack_top_priority = blk->symbol.empty() ? 5 :
          util::GetTokenPriority(blk->symbol.back().keyword_value);
      }
    }

    blk->symbol.emplace_back(request);

    blk->forward_priority = current_priority;
  }

  bool LineParser::CleanupStack(ParserBlock *blk) {
    bool result = true;

    while (!blk->symbol.empty() && !blk->symbol.back().IsPlaceholder()
      && blk->symbol.back().keyword_value != kKeywordBind) {
      ProduceVMCode(blk);
    }

    return result;
  }

  Message LineParser::Parse() {
    auto state = true;
    const auto size = tokens_.size();
    Message result;

    ParserBlock *blk = new ParserBlock();

    for (size_t i = 0; i < size; ++i) {
      if (!state)  break;

      blk->current = tokens_[i];
      blk->next = i + 1 < size ?
        tokens_[i + 1] : INVALID_TOKEN;
      blk->next_2 = i + 2 < size ?
        tokens_[i + 2] : INVALID_TOKEN;

      auto token_type = blk->current.second;
      if (token_type == kStringTypeSymbol) {
        Terminator value = util::GetTerminatorCode(blk->current.first);
        switch (value) {
        case kTerminatorAssign:         
          BindExpr(blk); 
          break;
        case kTerminatorComma:
          state = CleanupStack(blk);
          break;
        case kTerminatorDot:            
          DotExpr(blk); 
          break;
        case kTerminatorLeftParen:
          FuncInvokingExpr(blk);
          break;
        case kTerminatorLeftBracket:
          state = IndexExpr(blk);
          break;
        case kTerminatorLeftBrace:
          state = ArrayExpr(blk);
          break;
        case kTerminatorMonoOperator:  
          UnaryExpr(blk); 
          break;
        case kTerminatorBinaryOperator:         
          BinaryExpr(blk); 
          break;
        case kTerminatorRightSqrBracket:
        case kTerminatorRightBracket:
        case kTerminatorRightCurBracket:
          state = CleanupStack(blk);
          if (state) ProduceVMCode(blk);
          break;
        default:
          break;
        }
      }
      else if (token_type == kStringTypeIdentifier) {
        state = FunctionAndObject(blk);
      }
      else if (token_type == kStringTypeNull) {
        result = ERROR_MSG("Illegal token - " + blk->current.first)
          .SetIndex(index_);
        state = false;
      }
      else OtherToken(blk);
      blk->last = blk->current;
    }

    if (state) {
      while (!blk->symbol.empty()) {
        ProduceVMCode(blk);
      }
    }
    else {
      result = ERROR_MSG(error_string_).SetIndex(index_);
    }

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

  void LineParser::Clear() {
    tokens_.clear();
    tokens_.shrink_to_fit();
    action_base_.clear();
    action_base_.shrink_to_fit();
    error_string_.clear();
    index_ = 0;
  }

  Message LineParser::Make(CombinedCodeline &line) {
    vector<string> spilted_string = Scanning(line.second);
    Message msg = Tokenizer(spilted_string);

    if (msg.GetLevel() == kStateError) return msg;

    index_ = line.first;
    msg = Parse();

    if (msg.GetLevel() == kStateError) return msg;

    Request request(kKeywordSegment);
    ArgumentList args;

    int code = 
      static_cast<int>(util::GetKeywordCode(tokens_.front().first));

    args.emplace_back(
      Argument(to_string(code), kArgumentNormal, kStringTypeInt)
    );

    action_base_.emplace_front(std::make_pair(request, args));

    return msg;
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
    list<CombinedCodeline> script;
    LineParser line_parser;
    Message msg;
    VMCode anchorage;
    StateLevel level;
    Keyword ast_root;

    if (!ReadScript(script)) return false;

    auto end = script.end();
    for (auto it = script.begin(); it != end; ++it) {
      if (!good) break;
      msg = line_parser.Make(*it).SetIndex(it->first);
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

      if (find_in_vector(ast_root, nest_flag_collection)) {
        nest_.push(dest_->size());
        nest_origin_.push(it->first);
        nest_type_.push(ast_root);
      }

      if (ast_root == kKeywordEnd) {
        if (nest_type_.empty()) {
          trace::AddEvent("Invalid 'end' token");
          good = false;
          break;
        }
        if (compare_exp(nest_type_.top(), kKeywordWhile, kKeywordFor)) {
          anchorage.back().first.option.nest = nest_.top();
        }

        nest_.pop();
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
