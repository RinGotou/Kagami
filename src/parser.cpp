//#include <iostream>
#include <ctime>
#ifndef _NO_CUI_
#include <iostream>
#endif
#include "parser.h"

namespace kagami {
  namespace trace {
    vector<log_t> &GetLogger() {
      static vector<log_t> base;
      return base;
    }

    void Log(Message msg) {
      auto now = time(nullptr);
#if defined(_WIN32) && defined(_MSC_VER)
      char nowtime[30] = { ' ' };
      ctime_s(nowtime, sizeof(nowtime), &now);
      GetLogger().emplace_back(log_t(string(nowtime), msg));
#else
      string nowtime(ctime(&now));
      GetLogger().emplace_back(log_t(nowtime, msg));
#endif
    }
  }

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

  bool GetBooleanValue(string src) {
    if (src == kStrTrue) return true;
    if (src == kStrFalse) return false;
    if (src == "0" || src.empty()) return false;
    return true;
  }

  ScriptMachine::ScriptMachine(const char *target) {
    string temp;
    end = false;
    isTerminal = false;
    health = true;
    current = 0;
    size_t subscript = 0;

    stream.open(target, std::ios::in);
    if (stream.good()) {
      while (!stream.eof()) {
        std::getline(stream, temp);
        if (!IsBlankStr(temp) && temp.front() != '#') {
          storage.push_back(Processor().Build(temp).SetIndex(subscript));
        }
        subscript++;
      }
    }
    stream.close();
  }

  void ScriptMachine::ConditionRoot(bool value) {
    modeStack.push(currentMode);
    if (value == true) {
      entry::CreateManager();
      currentMode = kModeCondition;
      conditionStack.push(true);
    }
    else {
      currentMode = kModeNextCondition;
      conditionStack.push(false);
    }
  }

  void ScriptMachine::ConditionBranch(bool value) {
    if (!conditionStack.empty()) {
      if (conditionStack.top() == false && currentMode == kModeNextCondition
        && value == true) {
        entry::CreateManager();
        currentMode = kModeCondition;
        conditionStack.top() = true;
      }
    }
    else {
      //msg.combo
      health = false;
    }
  }

  void ScriptMachine::ConditionLeaf() {
    if (!conditionStack.empty()) {
      if (conditionStack.top() == true) {
        currentMode = kModeNextCondition;
      }
      else {
        entry::CreateManager();
        conditionStack.top() = true;
        currentMode = kModeCondition;
      }
    }
    else {
      //msg.combo
      health = false;
    }
  }

  void ScriptMachine::HeadSign(bool value, bool selfObjectManagement) {
    if (cycleNestStack.empty()) {
      modeStack.push(currentMode);
      if (!selfObjectManagement) entry::CreateManager();
    }
    else {
      if (cycleNestStack.top() != current - 1) {
        modeStack.push(currentMode);
        if (!selfObjectManagement) entry::CreateManager();
      }
    }
    if (value == true) {
      currentMode = kModeCycle;
      if (cycleNestStack.empty()) {
        cycleNestStack.push(current - 1);
      }
      else if (cycleNestStack.top() != current - 1) {
        cycleNestStack.push(current - 1);
      }
    }
    else if (value == false) {
      currentMode = kModeCycleJump;
      if (!cycleTailStack.empty()) {
        current = cycleTailStack.top();
      }
    }
    else {
      health = false;
    }
  }

  void ScriptMachine::TailSign() {
    if (currentMode == kModeCondition || currentMode == kModeNextCondition) {
      conditionStack.pop();
      currentMode = modeStack.top();
      modeStack.pop();
      entry::DisposeManager();
    }
    if (currentMode == kModeCycle || currentMode == kModeCycleJump) {
      switch (currentMode) {
      case kModeCycle:
        if (cycleTailStack.empty() || cycleTailStack.top() != current - 1) {
          cycleTailStack.push(current - 1);
        }
        current = cycleNestStack.top();
        entry::GetCurrentManager().clear();
        break;
      case kModeCycleJump:
        currentMode = modeStack.top();
        modeStack.pop();
        cycleNestStack.pop();
        if (!cycleTailStack.empty()) cycleTailStack.pop();
        entry::DisposeManager();
        break;
      default:break;
      }
    }
  }

  bool ScriptMachine::IsBlankStr(string target) {
    if (target == kStrEmpty || target.size() == 0) return true;
    for (const auto unit : target) {
      if (unit != ' ' || unit != '\n' || unit != '\t' || unit != '\r') {
        return false;
      }
    }
    return true;
  }

  Message ScriptMachine::Run() {
    Message result;

    currentMode = kModeNormal;
    nestHeadCount = 0;
    current = 0;
    health = true;

    if (storage.empty()) return result;

    entry::CreateManager();

    //Main state machine
    while (current < storage.size()) {
      if (!health) break;

      result = storage[current].Activiate(currentMode);
      const auto value = result.GetValue();
      const auto code  = result.GetCode();
      const auto selfObjectManagement = storage[current].IsSelfObjectManagement();

      if (value == kStrFatalError) {
        trace::Log(result.SetIndex(storage[current].GetIndex()));
        break;
      }

      if (value == kStrWarning) {
        trace::Log(result.SetIndex(storage[current].GetIndex()));
      }
      //TODO:return

      switch (code) {
      case kCodeConditionRoot:
        ConditionRoot(GetBooleanValue(value)); 
        break;
      case kCodeConditionBranch:
        if (nestHeadCount > 0) break;
        ConditionBranch(GetBooleanValue(value));
        break;
      case kCodeConditionLeaf:
        if (nestHeadCount > 0) break;
        ConditionLeaf();
        break;
      case kCodeHeadSign:
        HeadSign(GetBooleanValue(value), selfObjectManagement);
        break;
      case kCodeFillingSign:
        nestHeadCount++;
        break;
      case kCodeTailSign:
        if (nestHeadCount > 0) {
          nestHeadCount--;
          break;
        }
        TailSign();
        break;
      default:break;
      }
      ++current;
    }

    entry::DisposeManager();

    return result;
  }

  void ScriptMachine::Terminal() {
    Message msg;
    string buf;
    string head = kStrNormalArrow;
    Processor processor;
    bool subProcess = false;

    entry::CreateManager();

    while (msg.GetCode() != kCodeQuit) {
      std::cout << head;
      std::getline(std::cin, buf);
      processor.Build(buf);
      auto tokenValue = entry::GetGenericToken(processor.GetFirstToken().first);
      switch (tokenValue) {
      case GT_IF:
      case GT_WHILE:
      case GT_FOR:
        subProcess = true;
        head = kStrDotGroup;
        break;
      case GT_END:
        subProcess = false;
        storage.emplace_back(processor);
        head = kStrNormalArrow;
        msg = this->Run();
        storage.clear();
        current = 0;
        break;
      case GT_DEF:
        //
        break;
      default:
        break;
      }
      if (subProcess) {
        storage.emplace_back(processor);
      }
      else {
        msg = processor.Activiate();
      }

      if (msg.GetCode() < kCodeSuccess) {
        std::cout << msg.GetDetail() << std::endl;
        trace::Log(msg);
      }
    }

    entry::DisposeManager();
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
      auto type = kit.GetTokenType(toString(currentChar));
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

    while (kit.GetTokenType(toString(data.back())) == TokenTypeEnum::T_BLANK) {
      data.pop_back();
    }

    //Spilt
    forwardChar = 0;
    for (size_t count = 0; count < data.size(); ++count) {
      currentChar = data[count];
      if (currentChar == '\'' && forwardChar != '\\') {
        if (kit.GetTokenType(current) == TokenTypeEnum::T_BLANK) {
          current.clear();
        }
        stringProcessing = !stringProcessing;
      }
      current.append(1, currentChar);
      if (kit.GetTokenType(current) != TokenTypeEnum::T_NUL) {
        forwardChar = data[count];
        continue;
      }
      else {
        current = current.substr(0, current.size() - 1);
        if (kit.GetTokenType(current) == TokenTypeEnum::T_BLANK) {
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

    switch (kit.GetTokenType(current)) {
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
          if (kit.IsInteger(forward) && kit.IsInteger(next)) {
            output.back().append(current);
            appendingOnce = true;
            continue;
          }
        }
        if (current == "(" || current == "[") nest++;
        if (current == ")" || current == "]") nest--;
        if (current == "[") {
          if (kit.GetTokenType(forward) != TokenTypeEnum::T_GENERIC
            && forward != "]"
            && forward != ")") {
            errorString = "Illegal subscript operation.";
            nest = 0;
            health = false;
            break;
          }
        }
        if (current == ",") {
          if (kit.GetTokenType(forward) == TokenTypeEnum::T_SYMBOL &&
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

    if (stringProcessing == true) {
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
        token.second = kit.GetTokenType(unit);
        this->origin.push_back(token);
      }
    }

    kit.CleanupVector(origin).CleanupVector(output);

    return *this;
  }

  bool Processor::TakeAction(Message &msg) {
    deque<Object> parms;
    ObjectMap map;
    size_t idx = 0;
    Entry &ent = symbol.back();

    if (ent.GetId() == kStrNop) {
      /*Pending*/
      return true;
    }

    const size_t size = ent.GetParmSize();
    auto args = ent.GetArguments();
    idx = size;
    while (idx > 0 && !item.empty() && !item.back().IsPlaceholder()) {
      parms.push_front(item.back());
      item.pop_back();
      idx--;
    }
    if (!item.empty() && item.back().IsPlaceholder()) item.pop_back();

    for (idx = 0; idx < size; ++idx) {
      map.insert(pair<string, Object>(args[idx], parms[idx]));
    }

    auto flag = ent.GetFlag();
    if (flag == kFlagMethod) {
      map.insert(pair<string, Object>(kStrObject, item.back()));
      item.pop_back();
    }

    /*TODO:Execute*/
    GenericTokenEnum headEnt = symbol.front().GetTokenEnum(),
      currentEnum = ent.GetTokenEnum();
    switch (mode) {
    case kModeCycleJump:
      if (currentEnum == GT_END) {
        msg.combo(kStrEmpty, kCodeTailSign, kStrEmpty);
      }
      else if (headEnt == GT_IF || headEnt == GT_WHILE) {
        msg.combo(kStrRedirect, kCodeFillingSign, kStrTrue);
      }
      break;
    case kModeNextCondition:
      if (headEnt == GT_IF || headEnt == GT_WHILE) {
        msg.combo(kStrRedirect, kCodeFillingSign, kStrTrue);
      }
      else if (currentEnum == GT_ELSE) {
        msg.combo(kStrTrue, kCodeConditionLeaf, kStrEmpty);
      }
      else if (currentEnum == GT_END) {
        msg.combo(kStrEmpty, kCodeTailSign, kStrEmpty);
      }
      else if (currentEnum == GT_ELIF) {
        msg = ent.Start(map);
      }
      break;
    case kModeNormal:
    case kModeCondition:
    default:
      msg = ent.Start(map);
    }

    const auto code = msg.GetCode();
    const auto value = msg.GetValue();
    const auto detail = msg.GetDetail();

    if (code == kCodeObject) {
      auto object = msg.GetObj();
      item.push_back(object);
    }
    else if (value == kStrRedirect && code == kCodeSuccess || code == kCodeFillingSign) {
      item.push_back(Object()
        .Manage(detail)
        .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
        .SetTokenType(Kit().GetTokenType(detail)));
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
      auto typeId = item.back().GetTypeId();
      if (typeId == kTypeIdRawString) {
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
    if (forwardToken.second != TokenTypeEnum::T_GENERIC) {
      symbol.push_back(Entry(kStrNop));
    }
    symbol.push_back(Entry(currentToken.first));
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
      if (result == false) break;
    }
    if (result == true) {
      if (symbol.back().GetId() == "(") symbol.pop_back();
      result = TakeAction(msg);
    }
    return result;
  }

  bool Processor::FunctionAndObject(Message &msg) {
    bool function = false;
    bool result = true;
    bool defLine = false;

    if (currentToken.first == kStrDef) defLine = true;

    if (dotOperator) {
      string methods = item.back().GetMethods();
      bool isExisted = Kit().FindInStringGroup(currentToken.first, methods);
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
      if (defLine) {
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
            item.push_back(Object()
              .Manage(currentToken.first)
              .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
              .SetTokenType(currentToken.second));
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

    //TODO:function definition checking
    if (function && nextToken.first != "(" 
      && currentToken.first != kStrElse && currentToken.first != kStrEnd) {
      this->health = false;
      result = false;
      errorString = "Left bracket after function is missing";
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
    while (symbol.empty() != true) {
      if (symbol.back().GetId() == "(") {
        msg.combo(kStrFatalError, kCodeIllegalSymbol, "Right bracket is missing.");
        break;
      }
      TakeAction(msg);
    }
  }

  Message Processor::Activiate(size_t mode) {
    using namespace entry;
    Kit kit;
    Message result;
    auto state = true;
    const auto size = origin.size();

    if (health == false) {
      return Message(kStrFatalError, kCodeBadExpression, errorString);
    }

    this->mode = mode;
    lambdaObjectCount = 0;
    nextInsertSubscript = 0;
    operatorTargetType = kTypeIdNull;
    commaExpFunc = false,
    insertBetweenObject = false,
    disableSetEntry = false,
    dotOperator = false,
    subscriptProcessing = false;
    functionLine = false;
    defineLine = false;

    for (size_t i = 0; i < size; ++i) {
      if (!health) break;
      if (!state)  break;

      currentToken = origin[i];
      i < size - 1 ?
        nextToken = origin[i + 1] :
        nextToken = Token(kStrNull, TokenTypeEnum::T_NUL);

      result.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
      const auto tokenTypeEnum = currentToken.second;
      const auto tokenValue = currentToken.first;
      if (tokenTypeEnum == TokenTypeEnum::T_SYMBOL) {
        BasicTokenEnum value = GetBasicToken(tokenValue);
        switch (value) {
        case TOKEN_EQUAL: EqualMark(); break;
        case TOKEN_COMMA: break;
        case TOKEN_LEFT_SQRBRACKET: /*TODO:*/; break;
        case TOKEN_DOT:             dotOperator = true; break;
        case TOKEN_LEFT_BRACKET:    LeftBracket(result); break;
        case TOKEN_COLON:           state = Colon(); break;
        case TOKEN_RIGHT_SQRBRACKET:/*TODO:*/; break;
        case TOKEN_RIGHT_BRACKET:   state = RightBracket(result); break;
        case TOKEN_SELFOP:          /*TODO:*/ break;
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

    if (state == true) FinalProcessing(result);
    if (!health) result.combo(kStrFatalError, kCodeBadExpression, errorString);

    kit.CleanupDeque(item).CleanupDeque(symbol);
    nextToken = Token();
    forwardToken = Token();
    currentToken = Token();

    return result;
  }
}

