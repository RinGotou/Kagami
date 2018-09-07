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
    if (src == "{")   return TOKEN_LEFT_CURBRACKET;
    if (src == "}")   return TOKEN_RIGHT_CURBRACKET;
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
    if (data.front() == '#') return *this;

    while (kagami::Kit::GetTokenType(toString(data.back())) == TokenTypeEnum::T_BLANK) {
      data.pop_back();
    }

    //Spilt
    forwardChar = 0;
    bool delaySuspend = false;
    bool forwardEscChar = false;
    for (size_t count = 0; count < data.size(); ++count) {
      currentChar = data[count];
      if (delaySuspend) {
        stringProcessing = false;
        delaySuspend = false;
      }
      if (forwardEscChar) {
        origin.emplace_back(current);
        current.clear();
      }
      if (stringProcessing && forwardChar == '\\') {
        forwardEscChar = true;
      }
      else {
        forwardEscChar = false;
      }
      if (currentChar == '\'' && forwardChar != '\\') {
        if (!stringProcessing
          && Kit::GetTokenType(current) == TokenTypeEnum::T_BLANK) {
          current.clear();
        }
        stringProcessing ? delaySuspend = true : stringProcessing = true;
      }
      current.append(1, currentChar);
      auto tokenEnum = Kit::GetTokenType(current);
      if (Kit::GetTokenType(current) != TokenTypeEnum::T_NUL) {
        forwardChar = data[count];
        continue;
      }
      else {
        current = current.substr(0, current.size() - 1);
        if (Kit::GetTokenType(current) == TokenTypeEnum::T_BLANK) {
          if (stringProcessing) origin.emplace_back(current);
          current.clear();
          current.append(1, currentChar);
        }
        else {
          origin.emplace_back(current);
          current.clear();
          current.append(1, currentChar);
          if (!stringProcessing
            && Kit::GetTokenType(string().append(1, currentChar)) == T_BLANK) {
            continue;
          }
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
    bool PFlag = false;
    for (size_t count = 0; count < origin.size(); ++count) {
      if (count + 1 < origin.size()) next = origin[count + 1];

      current = origin[count];

      if (!stringProcessing) {
        //Pos/Neg sign
        if ((current == "+" || current == "-") && next != current && forward != current) {
          if (Kit::IsSymbol(forward) && Kit::IsInteger(next)) {
            PFlag = true;
          }
        }
        //combine decimal
        if (current == ".") {
          if (kagami::Kit::IsInteger(forward)
            && kagami::Kit::IsInteger(next)) {
            output.back().append(current);
            appendingOnce = true;
            continue;
          }
        }
        //bracket checking
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
        //comma checking
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

      //switching string processing mode
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
      //usual work
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
          else if ((forward == "+" || forward == "-") && PFlag) {
            output.back().append(current);
            PFlag = false;
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

  void Processor::Reversing(ProcCtlBlk *blk) {
    deque<Entry> *tempSymbol = new deque<Entry>();
    deque<Object> *tempObject = new deque<Object>();

    while (!blk->symbol.empty()
      && blk->symbol.back().GetPriority()
      == blk->symbol[blk->symbol.size() - 2].GetPriority()) {
      tempSymbol->push_back(blk->symbol.back());
      tempObject->push_back(blk->item.back());
      blk->symbol.pop_back();
      blk->item.pop_back();
    }
    tempSymbol->push_back(blk->symbol.back());
    blk->symbol.pop_back();
    int i = 2;
    while (i > 0) {
      tempObject->push_back(blk->item.back());
      blk->item.pop_back();
      --i;
    }
    while (!tempSymbol->empty()) {
      blk->symbol.push_back(tempSymbol->front());
      tempSymbol->pop_front();
    }
    while (!tempObject->empty()) {
      blk->item.push_back(tempObject->front());
      tempObject->pop_front();
    }
    delete tempSymbol;
    delete tempObject;
  }

  bool Processor::TakeAction(Message &msg, ProcCtlBlk *blk) {
    deque<Object> parms;
    ObjectMap objMap;
    size_t idx = 0;
    Entry &ent = blk->symbol.back();

    const size_t size = ent.GetParmSize();
    auto args = ent.GetArguments();
    auto mode = ent.GetArgumentMode();
    bool reversed = entry::IsOperatorToken(ent.GetTokenEnum()) && blk->needReverse;

    idx = size;
    bool doNotUseIdx = (mode == kCodeAutoSize);
    while (!blk->item.empty() && !blk->item.back().IsPlaceholder()) {
      if (!doNotUseIdx && idx == 0) break;
      if (reversed) parms.push_back(blk->item.back());
      else parms.push_front(blk->item.back());
      blk->item.pop_back();
      if (!doNotUseIdx) idx--;
    }
    if (!blk->item.empty() && blk->item.back().IsPlaceholder()) blk->item.pop_back();

    idx = 0;
    if (mode == kCodeAutoSize) {
      const size_t parmSize = size - 1;
      while (idx < parmSize) {
        objMap.insert(NamedObject(args[idx], parms[idx]));
        ++idx;
      }
      string argGroupHead = args.back();
      size_t count = 0;
      while (idx < parms.size()) {
        objMap.insert(NamedObject(argGroupHead + to_string(count), parms[idx]));
        idx++;
        count++;
      }
      objMap.insert(NamedObject("__size", Object()
        .Manage(to_string(parms.size()))
        .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
        .SetTokenType(T_INTEGER)));
    }
    else {
      while (idx < size) {
        if (idx >= parms.size() && ent.GetArgumentMode() == kCodeAutoFill) break;
        objMap.insert(NamedObject(args[idx], parms[idx]));
        ++idx;
      }
    }

    auto flag = ent.GetFlag();
    if (flag == kFlagMethod) {
      objMap.insert(NamedObject(kStrObject, blk->item.back()));
      parms.push_back(Object().SetArgSign(blk->item.back().GetOriginId()));
      blk->item.pop_back();
    }

    msg = ent.Start(objMap);
    const auto code = msg.GetCode();
    const auto value = msg.GetValue();
    const auto detail = msg.GetDetail();

    if (code == kCodeObject) {
      auto object = msg.GetObj();
      blk->item.emplace_back(object.SetRetSign());
    }
    else if (value == kStrRedirect && code == kCodeSuccess || code == kCodeHeadPlaceholder) {
      blk->item.emplace_back(Object()
        .Manage(detail)
        .SetRetSign()
        .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
        .SetTokenType(kagami::Kit::GetTokenType(detail)));
    }

    health = (value != kStrFatalError && code >= kCodeSuccess);
    instBase.push_back(Inst(ent, parms));
    blk->symbol.pop_back();
    return health;
  }

  Object *Processor::GetObj(string name, ProcCtlBlk *blk) {
    Object *result = nullptr;
    const auto ptr = entry::FindObject(name);
    if (ptr != nullptr) result = ptr;
    return result;
  }

  void Processor::EqualMark(ProcCtlBlk *blk) {
    if (!blk->item.empty()) {
      bool ref = blk->item.back().IsRef();
      if (!ref) {
        blk->symbol.push_back(entry::Order(kStrBind));
      }
      else {
        blk->symbol.push_back(entry::Order(kStrSet));
      }
    }
  }

  void Processor::LeftBracket(Message &msg, ProcCtlBlk *blk) {
    if (blk->defineLine) return;
    if (blk->forwardToken.second != TokenTypeEnum::T_GENERIC) {
      auto ent = entry::Order(kStrNop);
      blk->symbol.emplace_back(ent);
    }
    blk->symbol.emplace_back(blk->currentToken.first);
    blk->item.push_back(Object().SetPlaceholder());
  }

  bool Processor::RightBracket(Message &msg, ProcCtlBlk *blk) {
    bool result = true;
    deque<Entry> tempSymbol;
    deque<Object> tempObject;
    bool checked = false;

    while (!blk->symbol.empty() 
      && blk->symbol.back().GetId() != "(" 
      && blk->symbol.back().GetTokenEnum() != GT_BIND
      && blk->symbol.back().GetTokenEnum() != GT_SET) {

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

      result = TakeAction(msg, blk);
      if (!result) break;
    }
    if (result) {
      if (blk->needReverse) blk->needReverse = false;
      if (blk->symbol.back().GetId() == "(" || blk->symbol.back().GetId() == "[") blk->symbol.pop_back();
      result = TakeAction(msg, blk);
    }
    return result;
  }

  bool Processor::LeftSqrBracket(Message &msg, ProcCtlBlk *blk) {
    bool result = true;
    bool methodExisted = Kit::FindInStringGroup("__at", blk->item.back().GetMethods());
    if (methodExisted) {
      Entry ent = entry::Order("__at", blk->item.back().GetTypeId());
      if (ent.Good()) {
        blk->symbol.push_back(ent);
        blk->symbol.emplace_back("(");
        blk->item.push_back(Object().SetPlaceholder());
      }
      else {
        msg.combo(kStrFatalError, kCodeIllegalCall, "Function is not found - __at");
        result = false;
      }
    }
    return result;
  }

  bool Processor::SelfOperator(Message &msg, ProcCtlBlk *blk) {
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
      msg.combo(kStrFatalError, kCodeIllegalCall, "Unknown self operation");
      result = false;
    }
    return result;
  }

  bool Processor::LeftCurBracket(Message &msg, ProcCtlBlk *blk) {
    return true;
  }

  bool Processor::RightCurBracket(Message &msg, ProcCtlBlk *blk) {
    return true;
  }

  bool Processor::FunctionAndObject(Message &msg, ProcCtlBlk *blk) {
    bool function = false;
    bool result = true;

    if (blk->dotOperator) {
      string methods = blk->item.back().GetMethods();
      bool isExisted = Kit::FindInStringGroup(blk->currentToken.first, methods);
      if (isExisted) {
        auto ent = entry::Order(blk->currentToken.first, blk->item.back().GetTypeId());
        if (ent.Good()) {
          blk->symbol.push_back(ent);
          function = true;
        }
        /*TODO:find member*/
        else {
          result = false;
          msg.combo(kStrFatalError, kCodeIllegalCall, "Method or member is not found.");
        }
      }
      blk->dotOperator = false;
    }
    else {
      if (blk->defineLine) {
        blk->item.push_back(Object()
          .Manage(blk->currentToken.first)
          .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
          .SetTokenType(blk->currentToken.second));
      }
      else {
        if (blk->nextToken.first == "(") {
          auto ent = entry::Order(blk->currentToken.first);
          if (ent.Good()) {
            blk->symbol.push_back(ent);
          }
          else {
            result = false;
            msg.combo(kStrFatalError,
              kCodeIllegalCall,
              "Function is not found - " + blk->currentToken.first);
          }
        }
        else {
          if (blk->nextToken.first == "=") {
            Object *object = entry::FindObject(blk->currentToken.first);
            if (object != nullptr) {
              Object obj;
              obj.SetArgSign(blk->currentToken.first);
              obj.Ref(*object);
              blk->item.push_back(obj);
            }
            else {
              blk->item.push_back(Object()
                .Manage(blk->currentToken.first)
                .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
                .SetTokenType(blk->currentToken.second));
            }
          }
          else if (blk->currentToken.first == kStrEnd || blk->currentToken.first == kStrElse) {
            auto ent = entry::Order(blk->currentToken.first);
            blk->symbol.push_back(ent);
          }
          else if (blk->currentToken.first == kStrDef) {
            auto ent = entry::Order(blk->currentToken.first);
            blk->defineLine = true;
            blk->symbol.emplace_back(ent);
            blk->symbol.emplace_back("(");
            blk->item.push_back(Object().SetPlaceholder());
          }
          else {
            Object *object = entry::FindObject(blk->currentToken.first);
            if (object != nullptr) {
              Object obj;
              obj.SetArgSign(blk->currentToken.first);
              obj.Ref(*object);
              blk->item.push_back(obj);
            }
            else {
              result = false;
              msg.combo(kStrFatalError,
                kCodeIllegalCall,
                "Object is not found - " + blk->currentToken.first);
            }
          }
        }
      }
    }

    if (function && blk->nextToken.first != "("
      && blk->currentToken.first != kStrElse && blk->currentToken.first != kStrEnd) {
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

  void Processor::OtherToken(ProcCtlBlk *blk) {
    if (blk->insertBetweenObject) {
      blk->item.insert(blk->item.begin() + blk->nextInsertSubscript,
        Object().Manage(blk->currentToken.first)
        .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
        .SetTokenType(blk->currentToken.second));
      blk->insertBetweenObject = false;
    }
    else {
      blk->item.push_back(Object().Manage(blk->currentToken.first)
        .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
        .SetTokenType(blk->currentToken.second));
    }
  }

  void Processor::OtherSymbol(ProcCtlBlk *blk) {
    Entry ent = entry::Order(blk->currentToken.first);
    int currentPriority = ent.GetPriority();
    if (blk->symbol.empty()) {
      blk->symbol.push_back(ent);
    }
    else if (currentPriority < blk->symbol.back().GetPriority()) {
      auto j = blk->symbol.size() - 1;
      auto k = blk->item.size();
      while (blk->symbol[j].GetId() != "(" && blk->symbol[j].GetId() != "[" &&
        currentPriority < blk->symbol[j].GetPriority()) {
        k == blk->item.size() ? k -= 2 : k -= 1;
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

  void Processor::FinalProcessing(Message &msg, ProcCtlBlk *blk) {
    bool checked = false;
    while (!blk->symbol.empty()) {
      if (blk->symbol.back().GetId() == "(") {
        msg.combo(kStrFatalError, kCodeIllegalSymbol, "Right bracket is missing.");
        break;
      }
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
      if (!entry::IsOperatorToken(blk->symbol.back().GetTokenEnum())) {
        blk->needReverse = false;
      }
      if (!TakeAction(msg, blk)) break;
    }
  }

  Message Processor::RunWithCache() {
    deque<Object> retBase;
    vector<Inst>::iterator it = instBase.begin();
    Message msg;
    auto &manager = entry::GetCurrentManager();

    auto getObject = [&](Object &obj) -> Object{
      if (obj.IsRetSign()) {
        Object res = retBase.front();
        retBase.pop_front();
        return res;
      }
      if (obj.IsArgSign()) {
        return Object().Ref(*entry::FindObject(obj.GetOriginId()));
      }
      return obj;
    };

    for (; it != instBase.end(); it++) {
      ObjectMap objMap;
      auto &ent = it->first;
      auto &parms = it->second;
      size_t entParmSize = ent.GetParmSize();

      auto args = ent.GetArguments();
      auto mode = ent.GetArgumentMode();
      size_t idx = 0;
      if (mode == kCodeAutoSize) {
        const size_t parmSize = entParmSize - 1;
        while (idx < parmSize) {
          objMap.insert(NamedObject(args[idx], getObject(parms[idx])));
          ++idx;
        }
        string argGroupHead = args.back();
        size_t count = 0;
        while (idx < parms.size()) {
          objMap.insert(NamedObject(argGroupHead + to_string(count), getObject(parms[idx])));
          idx++;
          count++;
        }
        objMap.insert(NamedObject("__size", Object()
          .Manage(to_string(parms.size()))
          .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
          .SetTokenType(T_INTEGER)));
      }
      else {
        while (idx < entParmSize) {
          if (idx >= parms.size() && ent.GetArgumentMode() == kCodeAutoFill) break;
          objMap.insert(NamedObject(args[idx], getObject(parms[idx])));
          ++idx;
        }
      }

      if (ent.GetFlag() == kFlagMethod) {
        objMap.insert(NamedObject(kStrObject, getObject(parms.back())));
      }

      msg = ent.Start(objMap);
      const auto code = msg.GetCode();
      const auto value = msg.GetValue();
      const auto detail = msg.GetDetail();

      if (value == kStrFatalError) break;

      if (code == kCodeObject) {
        auto object = msg.GetObj();
        retBase.emplace_back(object);
      }
      else if (value == kStrRedirect && code == kCodeSuccess || code == kCodeHeadPlaceholder) {
        Object obj;
        obj.Manage(detail)
          .SetRetSign()
          .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
          .SetTokenType(kagami::Kit::GetTokenType(detail));
        if (entry::IsOperatorToken(ent.GetTokenEnum()) 
          && it + 1 != instBase.end()) {
          auto token = (it + 1)->first.GetTokenEnum();
          if (entry::IsOperatorToken(token)) {
            retBase.emplace_front(obj);
          }
          else {
            retBase.emplace_back(obj);
          }
        }
        else {
          retBase.emplace_back(obj);
        }
      }
    }
    return msg;
  }

  Message Processor::Activiate(size_t mode) {
    using namespace entry;
    Message result;
    auto state = true;
    const auto size = origin.size();
    
    if (!health) {
      return Message(kStrFatalError, kCodeBadExpression, errorString);
    }

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

    if (cached) {
      return RunWithCache();
    }

    ProcCtlBlk *blk = new ProcCtlBlk();
    blk->mode = mode;
    blk->lambdaObjectCount = 0;
    blk->nextInsertSubscript = 0;
    blk->operatorTargetType = kTypeIdNull;
    blk->insertBetweenObject = false;
    blk->dotOperator = false;
    blk->needReverse = false;
    blk->subscriptProcessing = false;
    blk->defineLine = false;

    for (size_t i = 0; i < size; ++i) {
      if (!health) break;
      if (!state)  break;

      blk->currentToken = origin[i];
      i < size - 1 ?
        blk->nextToken = origin[i + 1] :
        blk->nextToken = Token(kStrNull, TokenTypeEnum::T_NUL);

      result.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
      auto tokenTypeEnum = blk->currentToken.second;
      if (tokenTypeEnum == TokenTypeEnum::T_SYMBOL) {
        BasicTokenEnum value = GetBasicToken(blk->currentToken.first);
        switch (value) {
        case TOKEN_EQUAL: EqualMark(blk); break;
        case TOKEN_COMMA: break;
        case TOKEN_LEFT_SQRBRACKET: state = LeftSqrBracket(result, blk); break;
        case TOKEN_DOT:             blk->dotOperator = true; break;
        case TOKEN_LEFT_BRACKET:    LeftBracket(result, blk); break;
        case TOKEN_RIGHT_SQRBRACKET:state = RightBracket(result, blk); break;
        case TOKEN_RIGHT_BRACKET:   state = RightBracket(result, blk); break;
        case TOKEN_SELFOP:          state = SelfOperator(result, blk); break;
        case TOKEN_LEFT_CURBRACKET:break;
        case TOKEN_RIGHT_CURBRACKET:break;
        case TOKEN_OTHERS:          OtherSymbol(blk); break;
        default:break;
        }
      }
      else if (tokenTypeEnum == TokenTypeEnum::T_GENERIC) {
        state = FunctionAndObject(result, blk);
      }
      else if (tokenTypeEnum == TokenTypeEnum::T_NUL) {
        result.combo(kStrFatalError, kCodeIllegalParm, "Illegal token.");
        state = false;
      }
      else OtherToken(blk);
      blk->forwardToken = blk->currentToken;
    }

    if (state) {
      FinalProcessing(result, blk);
    }
    if (!health && result.GetDetail() == kStrEmpty) {
      result.combo(kStrFatalError, kCodeBadExpression, errorString);
    }

    Kit().CleanupDeque(blk->item).CleanupDeque(blk->symbol);
    blk->nextToken = Token();
    blk->forwardToken = Token();
    blk->currentToken = Token();
    delete blk;
    if (cached == false) {
      cached = true;
    }

    return result;
  }
}