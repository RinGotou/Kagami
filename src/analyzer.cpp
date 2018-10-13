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
    Kit kit;
    string current, temp;
    bool stringProcessing = false;
    bool delaySuspending = false;
    bool delaySwitching = false;
    bool escapeFlag = false;
    bool floatTokenProcessing = false;
    bool skipCurrentChar = false;
    char currentChar = 0, nextChar = 0, lastChar = 0;
    vector<string> output;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      currentChar = target[idx];

      (idx < target.size() - 1) ?
        nextChar = target[idx + 1] :
        nextChar = 0;

      if (delaySuspending) {
        stringProcessing = false;
        delaySuspending = false;
      }

      (stringProcessing && lastChar == '\\') ?
        escapeFlag = true :
        escapeFlag = false;

      if (currentChar == '\'' && !escapeFlag) {
        if (!stringProcessing && kit.GetTokenType(current) == T_BLANK) {
          current.clear();
        }

        stringProcessing ?
          delaySuspending = true :
          stringProcessing = true, delaySwitching = true;
      }

      if (!stringProcessing || delaySwitching) {
        temp = current;
        temp.append(1, currentChar);

        if (kit.GetTokenType(temp) == T_NUL) {
          auto type = kit.GetTokenType(current);
          switch (type) {
          case T_BLANK:
            current.clear();
            current.append(1, currentChar);
            break;
          case T_INTEGER:
            if (currentChar == '.' && isdigit(nextChar) != 0) {
              current.append(1, currentChar);
            }
            else {
              output.emplace_back(current);
              current.clear();
              current.append(1, currentChar);
            }
            break;
          default:
            output.emplace_back(current);
            current.clear();
            current.append(1, currentChar);
            break;
          }
        }
        else {
          current = temp;
        }

        delaySwitching ?
          delaySwitching = false :
          delaySwitching = delaySwitching;
      }
      else {
        escapeFlag ?
          currentChar = kit.GetEscapeChar(currentChar) :
          currentChar = currentChar;
        current.append(1, currentChar);
      }

      lastChar = target[idx];
    }

    if (kit.GetTokenType(current) != T_BLANK) {
      output.emplace_back(current);
    }

    return output;
  }

  Message Analyzer::Tokenizer(vector<string> target) {
    Kit kit;
    bool negativeFlag = false;
    vector<string> output;
    stack<string> bracketStack;
    Token current = Token("", T_NUL),
      next = Token("", T_NUL),
      last = Token("", T_NUL);
    Message msg;
    
    tokens.clear();
    health = true;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = Token(target[idx], kit.GetTokenType(target[idx]));
      (idx < target.size() - 1) ?
        next = Token(target[idx + 1], kit.GetTokenType(target[idx + 1])) :
        next = Token("", T_NUL);

      if (current.second == T_NUL) {
        msg = Message(kStrFatalError, kCodeBadExpression, "Unknown token - " + current.first + ".");
        break;
      }

      if (current.first == "+" || current.first == "-") {
        if (last.second == T_SYMBOL && (next.second == T_INTEGER || next.second == T_FLOAT)) {
          negativeFlag = true;
        }
      }

      if (current.first == "(" || current.first == "[" || current.first == "{") {
        bracketStack.push(current.first);
      }

      if (current.first == ")" || current.first == "]" || current.first == "}") {
        if (!bracketStack.empty() && bracketStack.top() != kBracketPairs.at(current.first)) {
          msg = Message(kStrFatalError, kCodeBadExpression, "Left bracket is missing.");
          break;
        }
        else {
          bracketStack.pop();
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

      if ((current.first == "+" || current.first == "-") && !tokens.empty()) {
        if (tokens.back().first == current.first) {
          tokens.back().first.append(current.first);
        }
        else {
          tokens.emplace_back(current);
        }
      }
      else if (negativeFlag && (last.first == "+" || last.first == "-")) {
        Token res = Token(last.first + current.first, current.second);
        tokens.back() = res;
      }
      else {
        tokens.emplace_back(current);
      }

      last = current;
    }

    return msg;
  }

  void Analyzer::Reversing(AnalyzerWorkBlock *blk) {
    deque<Entry> *tempSymbol = new deque<Entry>();
    deque<Argument> *tempParm = new deque<Argument>();

    while (!blk->symbol.empty()
      && blk->symbol.back().GetPriority()
      == blk->symbol[blk->symbol.size() - 2].GetPriority()) {
      tempSymbol->push_back(blk->symbol.back());

      tempParm->push_back(blk->args.back());
      blk->symbol.pop_back();
      blk->args.pop_back();
    }
    tempSymbol->push_back(blk->symbol.back());
    blk->symbol.pop_back();

    for (int count = 2; count > 0; count -= 1) {
      tempParm->push_back(blk->args.back());
      blk->args.pop_back();
    }

    while (!tempSymbol->empty()) {
      blk->symbol.push_back(tempSymbol->front());
      tempSymbol->pop_front();
    }
    while (!tempParm->empty()) {
      blk->args.push_back(tempParm->front());
      tempParm->pop_front();
    }
    delete tempSymbol;
    delete tempParm;
  }

  bool Analyzer::InstructionFilling(AnalyzerWorkBlock *blk) {
    deque<Argument> arguments;
    size_t idx = 0;
    Entry &ent = blk->symbol.back();

    const size_t size = ent.GetParmSize();
    auto args = ent.GetArguments();
    auto mode = ent.GetArgumentMode();
    auto token = ent.GetTokenEnum();
    bool reversed = entry::IsOperatorToken(ent.GetTokenEnum()) && blk->needReverse;

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
      arguments.emplace_back(Argument(blk->args.back().data, PT_OBJ, T_GENERIC));
      blk->args.pop_back();
    }

    actionBase.emplace_back(Instruction(ent, arguments));
    blk->symbol.pop_back();
    blk->args.emplace_back(Argument("", PT_RET, T_NUL));
    return health;
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
      Argument(blk->nextToken.first,PT_NORMAL,blk->nextToken.second)
    };
    actionBase.emplace_back(Instruction(ent, arguments));
  }

  void Analyzer::LeftBracket(AnalyzerWorkBlock *blk) {
    if (blk->defineLine) return;
    if (blk->forwardToken.second != TokenTypeEnum::T_GENERIC) {
      auto ent = entry::Order(kStrNop);
      blk->symbol.emplace_back(ent);
    }
    blk->symbol.emplace_back(blk->currentToken.first);
    blk->args.emplace_back(Argument());
  }

  bool Analyzer::RightBracket(AnalyzerWorkBlock *blk) {
    bool result = true;
    bool checked = false;

    string topId = blk->symbol.back().GetId();
    while (!blk->symbol.empty()
      && topId != "{" && topId != "[" && topId != "("
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
            blk->needReverse = true;
            Reversing(blk);
          }
        }
      }

      result = InstructionFilling(blk);
      if (!result) break;
      topId = blk->symbol.back().GetId();
    }
    if (result) {
      if (blk->needReverse) blk->needReverse = false;
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
      Argument("__at",PT_NORMAL,T_GENERIC)
    };
    actionBase.emplace_back(Instruction(ent, arguments));

    ent.SetRecheckInfo("__at", true);
    blk->symbol.emplace_back(ent);
    blk->symbol.emplace_back(blk->currentToken.first);
    blk->args.emplace_back(Argument());

    return result;
  }

  bool Analyzer::SelfOperator(AnalyzerWorkBlock *blk) {
    bool result = true;
    if (blk->forwardToken.second == T_GENERIC) {
      if (blk->currentToken.first == "++") {
        blk->symbol.push_back(entry::Order(kStrRightSelfInc));
      }
      else if (blk->currentToken.first == "--") {
        blk->symbol.push_back(entry::Order(kStrRightSelfDec));
      }
    }
    else if (blk->forwardToken.second != T_GENERIC && blk->nextToken.second == T_GENERIC) {
      if (blk->currentToken.first == "++") {
        blk->symbol.push_back(entry::Order(kStrLeftSelfInc));
      }
      else if (blk->currentToken.first == "--") {
        blk->symbol.push_back(entry::Order(kStrLeftSelfDec));
      }
    }
    else {
      errorString = "Unknown self operation";
      result = false;
    }
    return result;
  }

  bool Analyzer::LeftCurBracket(AnalyzerWorkBlock *blk) {
    bool result;
    if (blk->forwardToken.second == TokenTypeEnum::T_SYMBOL) {
      auto ent = entry::Order(kStrArray);
      blk->symbol.emplace_back(ent);
      blk->symbol.emplace_back(blk->currentToken.first);
      blk->args.emplace_back(Argument());
      result = true;
    }
    else {
      result = false;
      errorString = "Illegal curly bracket location.";
    }
    return result;
  }

  bool Analyzer::FunctionAndObject(AnalyzerWorkBlock *blk) {
    bool function = false;
    bool result = true;

    if (blk->defineLine) {
      blk->args.emplace_back(Argument(blk->currentToken.first, PT_NORMAL, T_GENERIC));
    }
    else {
      if (blk->nextToken.first == "=") {
        blk->args.emplace_back(Argument(blk->currentToken.first, PT_NORMAL, T_GENERIC));
      }
      else if (blk->nextToken.first == "(") {
        auto ent = entry::Order(blk->currentToken.first);
        bool needDomain = (blk->forwardToken.first == ".");
        if (needDomain) {
          ent.SetRecheckInfo(blk->currentToken.first, true);
        }
        else {
          if (!ent.Good()) {
            ent.SetRecheckInfo(blk->currentToken.first, false);
          }
        }
        blk->symbol.push_back(ent);
      }
      else {
        if (blk->currentToken.first == kStrEnd 
          || blk->currentToken.first == kStrElse
          || blk->currentToken.first == kStrContinue
          || blk->currentToken.first == kStrBreak) {
          auto ent = entry::Order(blk->currentToken.first);
          blk->symbol.push_back(ent);
        }
        else if (blk->currentToken.first == kStrDef) {
          auto ent = entry::Order(blk->currentToken.first);
          blk->defineLine = true;
          blk->symbol.emplace_back(ent);
          blk->symbol.emplace_back("(");
          blk->args.emplace_back(Argument());
        }
        else {
          blk->args.emplace_back(Argument(blk->currentToken.first, PT_OBJ, T_GENERIC));
        }
      }
    }

    if (function && blk->nextToken.first != "("
      && blk->currentToken.first != kStrElse 
      && blk->currentToken.first != kStrEnd
      && blk->currentToken.first != kStrFinally) {
      this->health = false;
      result = false;
      errorString = "Left bracket after function is missing";
    }

    if (blk->defineLine && blk->forwardToken.first == kStrDef && blk->nextToken.first != "(") {
      this->health = false;
      result = false;
      errorString = "Wrong definition pattern";
    }

    return result;
  }

  void Analyzer::OtherToken(AnalyzerWorkBlock *blk) {
    if (blk->insertBetweenObject) {
      blk->args.emplace(blk->args.begin() + blk->nextInsertSubscript,
        Argument(blk->currentToken.first, PT_NORMAL, blk->currentToken.second));
      blk->insertBetweenObject = false;
    }
    else {
      blk->args.emplace_back(
        Argument(blk->currentToken.first, PT_NORMAL, blk->currentToken.second));
    }
  }

  void Analyzer::OtherSymbol(AnalyzerWorkBlock *blk) {
    Entry ent = entry::Order(blk->currentToken.first);
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
      blk->nextInsertSubscript = k;
      blk->insertBetweenObject = true;
    }
    else {
      blk->symbol.push_back(ent);
    }
  }

  void Analyzer::FinalProcessing(AnalyzerWorkBlock *blk) {
    bool checked = false;
    while (!blk->symbol.empty()) {
      if (blk->symbol.back().GetId() == "(") {
        errorString = "Right bracket is missing";
        health = false;
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
            blk->needReverse = true;
            Reversing(blk);
          }
        }
      }
      if (!entry::IsOperatorToken(blk->symbol.back().GetTokenEnum())) {
        blk->needReverse = false;
      }
      if (!InstructionFilling(blk)) break;
    }
  }

  Message Analyzer::Parser() {
    using namespace entry;
    auto state = true;
    const auto size = tokens.size();
    Message result;

    AnalyzerWorkBlock *blk = new AnalyzerWorkBlock();
    blk->nextInsertSubscript = 0;
    blk->insertBetweenObject = false;
    blk->needReverse = false;
    blk->defineLine = false;

    for (size_t i = 0; i < size; ++i) {
      if (!health) break;
      if (!state)  break;

      blk->currentToken = tokens[i];
      i < size - 1 ?
        blk->nextToken = tokens[i + 1] :
        blk->nextToken = Token(kStrNull, TokenTypeEnum::T_NUL);

      auto tokenTypeEnum = blk->currentToken.second;
      if (tokenTypeEnum == TokenTypeEnum::T_SYMBOL) {
        BasicTokenEnum value = GetBasicToken(blk->currentToken.first);
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
      else if (tokenTypeEnum == TokenTypeEnum::T_GENERIC) {
        state = FunctionAndObject(blk);
      }
      else if (tokenTypeEnum == TokenTypeEnum::T_NUL) {
        result = Message(kStrFatalError, kCodeIllegalParm, "Illegal token.");
        state = false;
      }
      else OtherToken(blk);
      blk->forwardToken = blk->currentToken;
    }

    if (state) {
      FinalProcessing(blk);
    }
    if (!state || !health) {
      result = Message(kStrFatalError, kCodeBadExpression, errorString);
    }

    Kit().CleanupDeque(blk->args).CleanupDeque(blk->symbol);
    blk->nextToken = Token();
    blk->forwardToken = Token();
    blk->currentToken = Token();
    delete blk;
    return result;
  }

  void Analyzer::Clear() {
    Kit().CleanupVector(tokens).CleanupVector(actionBase);
    health = false;
    errorString.clear();
    index = 0;
  }

  Message Analyzer::Make(string target, size_t index) {
    vector<string> spiltedString = Scanning(target);
    Message msg = Tokenizer(spiltedString);
    this->index = index;
    if (msg.GetCode() >= kCodeSuccess) {
      msg = Parser();
    }
    return msg;
  }
}