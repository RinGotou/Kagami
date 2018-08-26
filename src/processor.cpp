#include "processor.h"

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
    return TOKEN_OTHERS;
  }

  Processor &Processor::Build(string target) {
    Kit kit;
    string current, data, forward, next;
    auto exemptBlankChar = true;
    auto stringProcessing = false;
    auto appendingOnce = false;
    char currentChar;
    auto forwardChar = ' ';
    vector<string> origin, output;
    size_t head = 0, tail = 0, nest = 0;
    auto toString = [](char t) ->string {return string().append(1, t); };
    health = true;

    //PreProcessing
    this->origin.clear();
    item.clear();
    symbol.clear();
    for (size_t count = 0; count < target.size(); ++count) {
      currentChar = target[count];
      auto type = kagami::Kit::GetTokenType(toString(currentChar));
      if (type != TokenTypeEnum::T_BLANK && exemptBlankChar) {
        head = count;
        exemptBlankChar = false;
      }
      if (currentChar == '\'' && forwardChar != '\\') stringProcessing = !stringProcessing;
      if (!stringProcessing && currentChar == '#') {
        tail = count;
        break;
      }
      forwardChar = target[count];
    }

    if (tail > head) data = target.substr(head, tail - head);
    else data = target.substr(head, target.size() - head);

    while (kagami::Kit::GetTokenType(toString(data.back())) == TokenTypeEnum::T_BLANK) {
      data.pop_back();
    }

    //Spilt
    forwardChar = 0;
    bool delaySuspend = false;
    for (size_t count = 0; count < data.size(); ++count) {
      currentChar = data[count];
      if (delaySuspend) stringProcessing = false;
      if (currentChar == '\'' && forwardChar != '\\') {
        if (!stringProcessing
          && Kit::GetTokenType(current) == TokenTypeEnum::T_BLANK) {
          current.clear();
        }
        stringProcessing ? delaySuspend = true : stringProcessing = true;
      }
      current.append(1, currentChar);
      if (kagami::Kit::GetTokenType(current) != TokenTypeEnum::T_NUL) {
        forwardChar = data[count];
        continue;
      }
      else {
        current = current.substr(0, current.size() - 1);
        if (kagami::Kit::GetTokenType(current) == TokenTypeEnum::T_BLANK) {
          if (stringProcessing) origin.emplace_back(current);
          current.clear();
          current.append(1, currentChar);
        }
        else {
          origin.emplace_back(current);
          current.clear();
          current.append(1, currentChar);
        }
      }
      forwardChar = data[count];
    }

    switch (kagami::Kit::GetTokenType(current)) {
    case TokenTypeEnum::T_NUL:
    case TokenTypeEnum::T_BLANK:
      break;
    default:
      origin.emplace_back(current);
      break;
    }
    current.clear();

    //third cycle
    stringProcessing = false;
    for (size_t count = 0; count < origin.size(); ++count) {
      if (count + 1 < origin.size()) next = origin[count + 1];

      current = origin[count];
      if (!stringProcessing) {
        if (current == ".") {
          if (kagami::Kit::IsInteger(forward)
            && kagami::Kit::IsInteger(next)) {
            output.back().append(current);
            appendingOnce = true;
            continue;
          }
        }
        if (current == "(" || current == "[") nest++;
        if (current == ")" || current == "]") nest--;
        if (current == "[") {
          if (kagami::Kit::GetTokenType(forward) != TokenTypeEnum::T_GENERIC
            && forward != "]"
            && forward != ")") {
            errorString = "Illegal subscript operation.";
            nest = 0;
            health = false;
            break;
          }
        }
        if (current == ",") {
          if (kagami::Kit::GetTokenType(forward) == TokenTypeEnum::T_SYMBOL &&
            forward != "]" && forward != ")" &&
            forward != "++" && forward != "--" &&
            forward != "'") {
            errorString = "Illegal comma location.";
            health = false;
            break;
          }
        }
      }
      if (current == "'" && forward != "\\") {
        if (stringProcessing) {
          output.back().append(current);
        }
        else {
          output.emplace_back(kStrEmpty);
          output.back().append(current);
        }
        stringProcessing = !stringProcessing;
      }
      else {
        if (stringProcessing) {
          output.back().append(current);
        }
        else if (appendingOnce) {
          output.back().append(current);
          appendingOnce = false;
        }
        else {
          if ((current == "+" || current == "-") && !output.empty()) {
            if (output.back() == current) {
              output.back().append(current);
            }
            else {
              output.emplace_back(current);
            }
          }
          else if (current == "=") {
            if (output.back() == "<" || output.back() == ">" || output.back() == "=" || output.back() == "!") {
              output.back().append(current);
            }
            else {
              output.emplace_back(current);
            }
          }
          else {
            output.emplace_back(current);
          }
        }
      }

      forward = origin[count];
    }

    if (stringProcessing) {
      errorString = "Quotation mark is missing.";
      health = false;
    }
    if (nest > 0) {
      errorString = "Bracket/Square Bracket is missing";
      health = false;
    }

    if (health) {
      origin.reserve(output.size());
      for (auto &unit : output) {
        Token token;
        token.first = unit;
        token.second = kagami::Kit::GetTokenType(unit);
        this->origin.push_back(token);
      }
    }

    kit.CleanupVector(origin).CleanupVector(output);

    return *this;
  }

  bool Processor::TakeAction(Message &msg) {
    deque<Object> parms;
    ObjectMap objMap;
    size_t idx = 0;
    Entry &ent = symbol.back();

    if (ent.GetId() == kStrNop) {
      auto rit = item.rbegin();
      while (rit != item.rend() && !rit->IsPlaceholder()) rit--;
      if (rit != item.rend()) item.erase(rit.base());
      return true;
    }

    const size_t size = ent.GetParmSize();
    auto args = ent.GetArguments();
    auto mode = ent.GetArgumentMode();
    bool reversed = entry::IsOperatorToken(ent.GetTokenEnum()) && needReverse;

    idx = size;
    while (idx > 0 && !item.empty() && !item.back().IsPlaceholder()) {
      if (reversed) parms.push_back(item.back());
      else parms.push_front(item.back());
      item.pop_back();
      if (mode !=kCodeAutoSize) idx--;
    }
    if (!item.empty() && item.back().IsPlaceholder()) item.pop_back();

    idx = 0;
    if (mode == kCodeAutoSize) {
      const size_t parmSize = size - 1;
      while (idx < parmSize) {
        objMap.insert(pair<string, Object>(args[idx], parms[idx]));
        ++idx;
      }
      string argGroupHead = args.back();
      size_t count = 0;
      while (idx < parms.size()) {
        objMap.insert(pair<string, Object>(argGroupHead + to_string(count), parms[idx]));
        idx++;
        count++;
      }
    }
    else {
      while (idx < size) {
        if (idx >= parms.size() && ent.GetArgumentMode() == kCodeAutoFill) break;
        objMap.insert(pair<string, Object>(args[idx], parms[idx]));
        ++idx;
      }
    }

    auto flag = ent.GetFlag();
    if (flag == kFlagMethod) {
      objMap.insert(pair<string, Object>(kStrObject, item.back()));
      item.pop_back();
    }

    msg = ent.Start(objMap);
    const auto code = msg.GetCode();
    const auto value = msg.GetValue();
    const auto detail = msg.GetDetail();

    if (code == kCodeObject) {
      auto object = msg.GetObj();
      item.emplace_back(object);
    }
    else if (value == kStrRedirect && code == kCodeSuccess || code == kCodeHeadPlaceholder) {
      item.emplace_back(Object()
        .Manage(detail)
        .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
        .SetTokenType(kagami::Kit::GetTokenType(detail)));
    }

    health = (value != kStrFatalError && code >= kCodeSuccess);
    symbol.pop_back();
    return health;
  }

  Object *Processor::GetObj(string name) {
    Object *result = nullptr;
    const auto ptr = entry::FindObject(name);
    if (ptr != nullptr) result = ptr;
    return result;
  }

  void Processor::EqualMark() {
    if (!item.empty()) {
      bool ref = item.back().IsRef();
      if (!ref) {
        symbol.push_back(entry::Order(kStrBind));
      }
      else {
        symbol.push_back(entry::Order(kStrSet));
      }
    }
  }

  bool Processor::Colon() {
    if (symbol.front().GetTokenEnum() == GenericTokenEnum::GT_FOR) {
      errorString = "Illegal colon location.";
      return false;
    }
    return true;
  }

  void Processor::LeftBracket(Message &msg) {
    if (defineLine) return;
    if (forwardToken.second != TokenTypeEnum::T_GENERIC) {
      symbol.emplace_back(kStrNop);
    }
    symbol.emplace_back(currentToken.first);
    item.push_back(Object().SetPlaceholder());
  }

  bool Processor::RightBracket(Message &msg) {
    bool result = true;
    deque<Entry> tempSymbol;
    deque<Object> tempObject;
    bool checked = false;

    while (!symbol.empty() && symbol.back().GetId() != "(") {
      if (!checked
        && symbol.back().GetTokenEnum() != GT_NUL
        && symbol[symbol.size() - 2].GetTokenEnum() != GT_NUL) {
        checked = true;
        needReverse = true;
        while (!symbol.empty()
          && symbol.back().GetPriority() == symbol[symbol.size() - 2].GetPriority()) {
          tempSymbol.push_back(symbol.back());
          tempObject.push_back(item.back());
          symbol.pop_back();
          item.pop_back();
        }
        tempSymbol.push_back(symbol.back());
        symbol.pop_back();
        int i = 2;
        while (i > 0) {
          tempObject.push_back(item.back());
          item.pop_back();
          --i;
        }
        while (!tempSymbol.empty()) {
          symbol.push_back(tempSymbol.front());
          tempSymbol.pop_front();
        }
        while (!tempObject.empty()) {
          item.push_back(tempObject.front());
          tempObject.pop_front();
        }
      }
      else if (checked
        && symbol.back().GetTokenEnum() != GT_NUL
        && symbol[symbol.size() - 2].GetTokenEnum() != GT_NUL) {
        if (symbol.back().GetPriority() != symbol[symbol.size() - 2].GetPriority()) {
          checked = false;
        }
      }

      result = TakeAction(msg);
      if (!result) break;
    }
    if (result) {
      if (needReverse) needReverse = false;
      if (symbol.back().GetId() == "(" || symbol.back().GetId() == "[") symbol.pop_back();
      result = TakeAction(msg);
    }
    return result;
  }

  bool Processor::LeftSqrBracket(Message &msg) {
    bool result = true;
    bool methodExisted = Kit::FindInStringGroup("__at", item.back().GetMethods());
    if (methodExisted) {
      Entry ent = entry::Order("__at", item.back().GetTypeId());
      if (ent.Good()) {
        symbol.push_back(ent);
        symbol.emplace_back("(");
        item.push_back(Object().SetPlaceholder());
      }
      else {
        msg.combo(kStrFatalError, kCodeIllegalCall, "Function is not found - __at");
        result = false;
      }
    }
    return result;
  }

  bool Processor::SelfOperator(Message &msg) {
    bool result = true;
    if (forwardToken.second == T_GENERIC) {
      if (currentToken.first == "++") {
        symbol.push_back(entry::Order(kStrRightSelfInc));
      }
      else if (currentToken.first == "--") {
        symbol.push_back(entry::Order(kStrRightSelfDec));
      }
    }
    else if (forwardToken.second != T_GENERIC && nextToken.second == T_GENERIC) {
      if (currentToken.first == "++") {
        symbol.push_back(entry::Order(kStrLeftSelfInc));
      }
      else if (currentToken.first == "--") {
        symbol.push_back(entry::Order(kStrLeftSelfDec));
      }
    }
    else {
      msg.combo(kStrFatalError, kCodeIllegalCall, "Unknown self operation");
      result = false;
    }
    return result;
  }

  bool Processor::FunctionAndObject(Message &msg) {
    bool function = false;
    bool result = true;

    if (dotOperator) {
      string methods = item.back().GetMethods();
      bool isExisted = Kit::FindInStringGroup(currentToken.first, methods);
      if (isExisted) {
        auto ent = entry::Order(currentToken.first, item.back().GetTypeId());
        if (ent.Good()) {
          symbol.push_back(ent);
          function = true;
        }
        /*TODO:find member*/
        else {
          result = false;
          msg.combo(kStrFatalError, kCodeIllegalCall, "Method or member is not found.");
        }
      }
      dotOperator = false;
    }
    else {
      if (defineLine) {
        item.push_back(Object()
          .Manage(currentToken.first)
          .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
          .SetTokenType(currentToken.second));
      }
      else {
        if (nextToken.first == "(") {
          auto ent = entry::Order(currentToken.first);
          if (ent.Good()) {
            symbol.push_back(ent);
          }
          else {
            result = false;
            msg.combo(kStrFatalError,
              kCodeIllegalCall,
              "Function is not found - " + currentToken.first);
          }
        }
        else {
          if (nextToken.first == "=") {
            Object *object = entry::FindObject(currentToken.first);
            if (object != nullptr) {
              item.push_back(Object().Ref(*object));
            }
            else {
              item.push_back(Object()
                .Manage(currentToken.first)
                .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
                .SetTokenType(currentToken.second));
            }
          }
          else if (currentToken.first == kStrEnd || currentToken.first == kStrElse) {
            auto ent = entry::Order(currentToken.first);
            symbol.push_back(ent);
          }
          else if (currentToken.first == kStrDef) {
            auto ent = entry::Order(currentToken.first);
            defineLine = true;
            symbol.emplace_back(ent);
            symbol.emplace_back("(");
            item.push_back(Object().SetPlaceholder());
          }
          else {
            Object *object = entry::FindObject(currentToken.first);
            if (object != nullptr) {
              item.push_back(Object().Ref(*object));
            }
            else {
              result = false;
              msg.combo(kStrFatalError,
                kCodeIllegalCall,
                "Object is not found - " + currentToken.first);
            }
          }
        }
      }
    }

    if (function && nextToken.first != "("
      && currentToken.first != kStrElse && currentToken.first != kStrEnd) {
      this->health = false;
      result = false;
      errorString = "Left bracket after function is missing";
    }

    if (defineLine && forwardToken.first == kStrDef && nextToken.first != "(") {
      this->health = false;
      result = false;
      errorString = "Wrong definition pattern";
    }

    return result;
  }

  void Processor::OtherToken() {
    if (insertBetweenObject) {
      item.insert(item.begin() + nextInsertSubscript,
        Object().Manage(currentToken.first)
        .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
        .SetTokenType(currentToken.second));
      insertBetweenObject = false;
    }
    else {
      item.push_back(Object().Manage(currentToken.first)
        .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
        .SetTokenType(currentToken.second));
    }
  }

  void Processor::OtherSymbol() {
    Entry ent = entry::Order(currentToken.first);
    int currentPriority = ent.GetPriority();
    if (symbol.empty()) {
      symbol.push_back(ent);
    }
    else if (currentPriority < symbol.back().GetPriority()) {
      auto j = symbol.size() - 1;
      auto k = item.size();
      while (symbol[j].GetId() != "(" && symbol[j].GetId() != "[" &&
        currentPriority < symbol[j].GetPriority()) {
        k == item.size() ? k -= 2 : k -= 1;
        --j;
      }
      symbol.insert(symbol.begin() + j + 1, ent);
      nextInsertSubscript = k;
      insertBetweenObject = true;
    }
    else {
      symbol.push_back(ent);
    }
  }

  void Processor::FinalProcessing(Message &msg) {
    while (!symbol.empty()) {
      if (symbol.back().GetId() == "(") {
        msg.combo(kStrFatalError, kCodeIllegalSymbol, "Right bracket is missing.");
        break;
      }
      TakeAction(msg);
    }
  }

  Message Processor::Activiate(size_t mode) {
    using namespace entry;
    Message result;
    auto state = true;
    const auto size = origin.size();

    if (!health) {
      return Message(kStrFatalError, kCodeBadExpression, errorString);
    }

    this->mode = mode;
    auto token = entry::GetGenericToken(origin.front().first);
    switch (mode) {
    case kModeDef:
      if (token == GT_WHILE || token == GT_IF) {
        return Message(kStrRedirect, kCodeHeadPlaceholder, kStrTrue);
      }
      else if (token != GT_END) {
        return Message(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
      }
      break;
    case kModeNextCondition:
      if (token == GT_IF || token == GT_WHILE) {
        return Message(kStrRedirect, kCodeHeadPlaceholder, kStrTrue);
      }
      else if (token != GT_ELSE && token != GT_END && token != GT_ELIF) {
        return Message(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
      }
      break;
    case kModeCycleJump:
      if (token != GT_END && token != GT_IF && token != GT_WHILE) {
        return Message(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
      }
    default:break;
    }

    lambdaObjectCount = 0;
    nextInsertSubscript = 0;
    operatorTargetType = kTypeIdNull;
    insertBetweenObject = false;
    dotOperator = false;
    needReverse = false;
    subscriptProcessing = false;
    defineLine = false;

    for (size_t i = 0; i < size; ++i) {
      if (!health) break;
      if (!state)  break;

      currentToken = origin[i];
      i < size - 1 ?
        nextToken = origin[i + 1] :
        nextToken = Token(kStrNull, TokenTypeEnum::T_NUL);

      result.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
      auto tokenTypeEnum = currentToken.second;
      if (tokenTypeEnum == TokenTypeEnum::T_SYMBOL) {
        BasicTokenEnum value = GetBasicToken(currentToken.first);
        switch (value) {
        case TOKEN_EQUAL: EqualMark(); break;
        case TOKEN_COMMA: break;
        case TOKEN_LEFT_SQRBRACKET: state = LeftSqrBracket(result); break;
        case TOKEN_DOT:             dotOperator = true; break;
        case TOKEN_LEFT_BRACKET:    LeftBracket(result); break;
        case TOKEN_COLON:           state = Colon(); break;
        case TOKEN_RIGHT_SQRBRACKET:state = RightBracket(result); break;
        case TOKEN_RIGHT_BRACKET:   state = RightBracket(result); break;
        case TOKEN_SELFOP:          state = SelfOperator(result); break;
        case TOKEN_OTHERS:          OtherSymbol(); break;
        default:break;
        }
      }
      else if (tokenTypeEnum == TokenTypeEnum::T_GENERIC) {
        state = FunctionAndObject(result);
      }
      else if (tokenTypeEnum == TokenTypeEnum::T_NUL) {
        result.combo(kStrFatalError, kCodeIllegalParm, "Illegal token.");
        state = false;
      }
      else OtherToken();
      forwardToken = currentToken;
    }

    if (state) {
      FinalProcessing(result);
    }
    if (!health && result.GetDetail() == kStrEmpty) {
      result.combo(kStrFatalError, kCodeBadExpression, errorString);
    }

    item.clear();
    symbol.clear();
    nextToken = Token();
    forwardToken = Token();
    currentToken = Token();

    return result;
  }
}