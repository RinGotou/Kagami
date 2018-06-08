//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Suzu Nakamura
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//*Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//#include <iostream>
#include <ctime>
#include "parser.h"

namespace kagami {
  namespace trace {
    vector<log_t> &GetLogger() {
      static vector<log_t> base;
      return base;
    }

    void Log(Message msg) {
      auto now = time(nullptr);
#if defined(_WIN32)
      char nowtime[30] = { ' ' };
      ctime_s(nowtime, sizeof(nowtime), &now);
      GetLogger().push_back(log_t(string(nowtime), msg));
#else
      string nowtime(ctime(&now));
      GetLogger().push_back(log_t(nowtime, msg));
#endif
    }

    bool IsEmpty() {
      return GetLogger().empty();
    }
  }

  namespace entry {
    vector<EntryProvider> &GetEntryBase() {
      static vector<EntryProvider> base;
      return base;
    }

    void Inject(EntryProvider provider) {
      GetEntryBase().emplace_back(provider);
    }

    EntryProvider Order(string id,string type = kTypeIdNull,int size = -1) {
      EntryProvider result;
      vector<EntryProvider> &base = GetEntryBase();
      string spectype = kTypeIdNull;
      //size_t argsize = 0;

      if (id == "+" || id == "-" || id == "*" || id == "/"
        || id == "==" || id == "<=" || id == ">=" || id == "!="
        || id == ">" || id == "<") {
        result = Order("binexp");
      }
      else if (id == "=") {
        result = Order(kStrSet);
      }
      else {
        for (auto &unit : base) {
          if (unit.GetId() == id) {
            if (type == unit.GetSpecificType() && (size == -1 || size == unit.GetArgumentSize())) {
              result = unit;
              break;
            }
          }
        }
      }
      return result;
    }

    size_t GetRequiredCount(string target) {
      size_t result = 0;
      if (target == "+" || target == "-" || target == "*" || target == "/"
        || target == "==" || target == "<=" || target == ">=" || target == "!=") {
        result = Order("binexp").GetArgumentSize();
      }
      else {
        auto provider = Order(target);
        if(provider.Good()) {
          result = provider.GetArgumentSize();
        }
      }
      return result;
    }

    void RemoveByTemplate(ActivityTemplate temp) {
      auto &base = GetEntryBase();
      vector<EntryProvider>::iterator it;

      for (it = base.begin(); it != base.end(); ++it) {
        if (*it == temp) {
          break;
        }
      }
      if (it != base.end()) {
        base.erase(it);
      }
    }
  }

  ScriptProvider::ScriptProvider(const char *target) {
    string temp;
    end = false;
    health = false;
    current = 0;

    stream.open(target, std::ios::in);
    if (stream.good()) {
      while (!stream.eof()) {
        std::getline(stream, temp);
        if (!IsBlankStr(temp) && temp.front() != '#') {
          storage.push_back(Processor().Reset().Build(temp));
          if (!storage.back().IsHealth()) {
            //this->errorString = storage.back().getErrorString();
          }
        }
      }
      if (!storage.empty()) health = true;
    }
    stream.close();
  }

  Message ScriptProvider::Run(deque<string> res) {
    Message result;
    stack<size_t> cycleNestStack;
    stack<size_t> cycleTailStack;
    stack<bool> conditionStack;
    stack<size_t> modeStack;
    size_t currentMode = kModeNormal;
    size_t nestHeadCount = 0;
    auto health = true;

    if (storage.empty()) {
      return result;
    }

    entry::CreateManager();

    //TODO:add custom function support
    if (!res.empty()) {
      if (res.size() != parameters.size()) {
        result.combo(kStrFatalError, kCodeIllegalCall, "wrong parameter count.");
        return result;
      }
      for (size_t count = 0; count < parameters.size(); count++) {
        //CreateObject(parameter.at(i), res.at(i), false);
      }
    }

    //Main state machine
    while (current < storage.size()) {
      if (!health) break;
      result = storage[current].Start(currentMode);
      const auto value = result.GetValue();
      const auto code = result.GetCode();

      if (value == kStrFatalError) {
        trace::Log(result);
        break;
      }
      if (value == kStrWarning) {
        trace::Log(result);
      }
      //TODO:return

      switch (code) {
      case kCodeConditionRoot:
        modeStack.push(currentMode);
        if (value == kStrTrue) {
          currentMode = kModeCondition;
          conditionStack.push(true);
        }
        else if (value == kStrFalse) {
          currentMode = kModeNextCondition;
          conditionStack.push(false);
        }
        else {
          health = false;
        }
        break;
      case kCodeConditionBranch:
        if (nestHeadCount > 0) break;
        if (!conditionStack.empty()) {
          if (conditionStack.top() == false && currentMode == kModeNextCondition
            && value == kStrTrue) {
            currentMode = kModeCondition;
            conditionStack.top() = true;
          }
        }
        else {
          //msg.combo
          health = false;
        }
        break;
      case kCodeConditionLeaf:
        if (nestHeadCount > 0) break;
        if (!conditionStack.empty()) {
          switch (conditionStack.top()) {
          case true:
            currentMode = kModeNextCondition;
            break;
          case false:
            conditionStack.top() = true;
            currentMode = kModeCondition;
            break;
          }
        }
        else {
          //msg.combo
          health = false;
        }
        break;
      case kCodeHeadSign:
        if (cycleNestStack.empty()) {
          modeStack.push(currentMode);
        }
        else {
          if (cycleNestStack.top() != current - 1) {
            modeStack.push(currentMode);
          }
        }
        if (value == kStrTrue) {
          currentMode = kModeCycle;
          if (cycleNestStack.empty()) {
            cycleNestStack.push(current - 1);
          }
          else if (cycleNestStack.top() != current - 1) {
            cycleNestStack.push(current - 1);
          }
        }
        else if (value == kStrFalse) {
          currentMode = kModeCycleJump;
          if (!cycleTailStack.empty()) {
            current = cycleTailStack.top();
          }
        }
        else {
          health = false;
        }
        break;
      case kCodeFillingSign:
        nestHeadCount++;
        break;
      case kCodeTailSign:
        if (nestHeadCount > 0) {
          nestHeadCount--;
          break;
        }
        if (currentMode == kModeCondition || currentMode == kModeNextCondition) {
          conditionStack.pop();
          currentMode = modeStack.top();
          modeStack.pop();
        }
        if (currentMode == kModeCycle || currentMode == kModeCycleJump) {
          switch (currentMode) {
          case kModeCycle:
            if (cycleTailStack.empty() || cycleTailStack.top() != current - 1) {
              cycleTailStack.push(current - 1);
            }
            current = cycleNestStack.top();
            break;
          case kModeCycleJump:
            currentMode = modeStack.top();
            modeStack.pop();
            cycleNestStack.pop();
            cycleTailStack.pop();
            break;
          default:break;
          }
        }
      default:break;
      }
      ++current;
    }

    entry::DisposeManager();

    return result;
  }

  Processor &Processor::Build(string target) {
    Kit kit;
    string current, data, forward;
    auto exemptBlankChar = true;
    auto stringProcessing = false;
    char currentChar;
    auto forwardChar = ' ';
    vector<string> origin, output;
    size_t head = 0, tail = 0, nest = 0;
    auto toString = [](char t) ->string {return string().append(1, t); };
    health = true;

    if (target.front() == '#') return *this;

    //PreProcessing
    for (size_t count = 0; count < target.size(); ++count) {
      currentChar = target[count];
      if (kit.GetDataType(toString(currentChar)) != kTypeBlank
        && exemptBlankChar) {
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

    while (kit.GetDataType(toString(data.back())) == kTypeBlank) data.pop_back();
    
    //Spilt
    forwardChar = 0;
    for (size_t count = 0; count < data.size(); ++count) {
      currentChar = data[count];
      if (currentChar == '\'' && forwardChar != '\\') {
        if (kit.GetDataType(current) == kTypeBlank) {
          current.clear();
        }
        stringProcessing = !stringProcessing;
      }
      current.append(1, currentChar);
      if (kit.GetDataType(current) != kTypeNull) {
        forwardChar = data[count];
        continue;
      }
      else {
        current = current.substr(0, current.size() - 1);
        if (kit.GetDataType(current) == kTypeBlank) {
          if (stringProcessing) origin.push_back(current);
          current.clear();
          current.append(1, currentChar);
        }
        else {
          origin.push_back(current);
          current.clear();
          current.append(1, currentChar);
        }
      }
      forwardChar = data[count];
    }

    const auto type = kit.GetDataType(current);
    if (type != kTypeNull && type != kTypeBlank) origin.push_back(current);
    current.clear();

    //third cycle
    stringProcessing = false;
    for (size_t count = 0; count < origin.size(); ++count) {
      current = origin[count];
      if (!stringProcessing) {
        if (current == "(" || current == "[") nest++;
        if (current == ")" || current == "]") nest--;
        if (current == "[") {
          if (kit.GetDataType(forward) != kGenericToken && forward != "]" && forward != ")") {
            errorString = "Illegal subscript operation.";
            nest = 0;
            health = false;
            break;
          }
        }
        if (current == ",") {
          if (kit.GetDataType(forward) == kTypeSymbol &&
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
          output.push_back(kStrEmpty);
          output.back().append(current);
        }
        stringProcessing = !stringProcessing;
      }
      else {
        if (stringProcessing) {
          output.back().append(current);
        }
        else {
          if ((current == "+" || current == "-") && !output.empty()) {
            if (output.back() == current) {
              output.back().append(current);
            }
            else {
              output.push_back(current);
            }
          }
          else if (current == "=") {
            if (output.back() == "<" || output.back() == ">" || output.back() == "=" || output.back() == "!") {
              output.back().append(current);
            }
            else {
              output.push_back(current);
            }
          }
          else {
            output.push_back(current);
          }
        }
      }

      forward = origin[count];
    }

    if(health) {
      for(auto &unit : output) {
        types.emplace_back(kit.GetDataType(unit));
      }
    }

    if (stringProcessing == true) {
      errorString = "Quotation mark is missing.";
      health = false;
    }
    if (nest > 0) {
      errorString = "Bracket/Square Bracket is missing";
      health = false;
    }

    raw = output;

    return *this;
  }

  int Processor::GetPriority(const string target) {
    int priority;
    if (target == "=" || target == "var") {
      priority = 0;
    }
    else if (target == "==" || target == ">=" || target == "<="
      || target == "!=" || target == "<" || target == ">") {
      priority = 1;
    }
    else if (target == "+" || target == "-") {
      priority = 2;
    }
    else if (target == "*" || target == "/" || target == "\\") {
      priority = 3;
    }
    else {
      priority = 4;
    }
    return priority;
  }

  Object *Processor::GetObj(string name) {
    Object *result = nullptr;
    if (name.substr(0, 2) == "__") {
      const auto it = lambdamap.find(name);
      if (it != lambdamap.end()) result = &(it->second);
    }
    else {
      const auto ptr = entry::FindObject(name);
      if (ptr != nullptr) result = ptr;
    }
    return result;
  }

  vector<string> Processor::Spilt(string target) {
    string temp;
    auto start = false;
    for (auto &unit : target) {
      if (unit == ':') {
        start = true;
        continue;
      }
      if (start) {
        temp.append(1, unit);
      }
    }
    auto result = Kit().BuildStringVector(temp);
    return result;
  }

  string Processor::GetHead(string target) {
    string result;
    for (auto &unit : target) {
      if (unit == ':') break;
      else result.append(1, unit);
    }
    return result;
  }

  bool Processor::Assemble(Message &msg) {
    Kit kit;
    Attribute attribute;
    Object temp, *origin;
    size_t count;
    deque<string> tokens;
    vector<size_t> tokenTypes;
    ObjectMap map;
    auto providerSize = -1;
    auto health = true;
    auto providerType = kTypeIdNull;
    auto tags = Spilt(symbol.back());
    const auto id = GetHead(symbol.back());
    const Attribute strAttr(type::GetTemplate(kTypeIdRawString)->GetMethods(), false);


    auto getName = [](string target) ->string {
      if (target.front() == '&' || target.front() == '%')
        return target.substr(1, target.size() - 1);
      else return target;
    };

    if (id == kStrNop) return true;

    if (!tags.empty()) {
      providerType = tags.front();
      if (tags.size() > 1) {
        providerSize = stoi(tags[1]);
      }
      else {
        providerSize = -1;
      }
    }

    auto provider = entry::Order(id, providerType, providerSize);

    if (!provider.Good()) {
      msg.combo(kStrFatalError, kCodeIllegalCall, "Activity not found.");
      return false;
    }

    const auto size = provider.GetArgumentSize();
    const auto argMode = provider.GetArgumentMode();
    const auto priority = provider.GetPriority();
    auto args = provider.GetArguments();
    
    if (!disableSetEntry) {
      count = size;
      while (count > 0 && !item.empty() && item.back() != "(") {
        tokens.push_front(item.back());
        item.pop_back();
        count--;
      }
      if (!item.empty() && item.back() == "(") item.pop_back();
    }
    else {
      while (item.back() != "," && !item.empty()) {
        tokens.push_front(item.back());
        item.pop_back();
      }
    }

    for (auto &unit : tokens) {
      tokenTypes.emplace_back(kit.GetDataType(unit));
    }

    if (argMode == kCodeNormalArgs && (tokens.size() < size || tokens.size() > size)) {
      msg.combo(kStrFatalError, kCodeIllegalArgs, "Parameter doesn't match expected count.(01)");
      health = false;
    }
    else if (argMode == kCodeAutoFill && tokens.size() < 1) {
      msg.combo(kStrFatalError, kCodeIllegalArgs, "You should provide at least one parameter.(02)");
      health = false;
    }
    else {
      for (count = 0; count < size; count++) {
        if (tokens.size() - 1 < count) {
          map.insert(pair<string, Object>(getName(args[count]), Object()));
        }
        else {
          const auto tokenType = tokenTypes[count];
          switch (tokenType) {
          case kGenericToken:
            if (args[count].front() == '&') {
              origin = GetObj(tokens[count]);
              temp.Ref(*origin);
            }
            else if (args[count].front() == '%') {
              temp.Manage(tokens[count], kTypeIdRawString, kit.BuildAttrStr(strAttr));
            }
            else {
              temp = *GetObj(tokens[count]);
            }
            break;
          default:
            temp.Manage(tokens[count], kTypeIdRawString, kit.BuildAttrStr(strAttr));
            break;
          }
          map.insert(pair<string,Object>(getName(args[count]), temp));
          temp.Clear();
        }
      }

      switch (priority) {
      case kFlagOperatorEntry:
        map.insert(pair<string, Object>(kStrOperator, Object()
          .Manage(symbol.back(), kTypeIdRawString, kit.BuildAttrStr(strAttr))));
        break;
      case kFlagMethod:
        origin = GetObj(item.back());
        map.insert(pair<string, Object>(kStrObject, Object()
          .Ref(*origin)));
        item.pop_back();
        break;
      default:
        break;
      }
    }
    
    if (health) {
      switch (mode) {
      case kModeCycleJump:
        if (id == kStrEnd) msg = provider.Start(map);
        else if (symbol.front() == kStrIf || symbol.front() == kStrWhile) {
          msg.combo(kStrRedirect, kCodeFillingSign, kStrTrue);
        }
        break;
      case kModeNextCondition:
        if (symbol.front() == kStrIf || symbol.front() == kStrWhile) {
          msg.combo(kStrRedirect, kCodeFillingSign, kStrTrue);
        }
        else if (id == kStrElse || id == kStrEnd) msg = provider.Start(map);
        else if (symbol.front() == kStrElif) msg = provider.Start(map);
        else msg.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
        break;
      case kModeNormal:
      case kModeCondition:
      default:
        msg = provider.Start(map);
        break;
      }

      const auto code = msg.GetCode();
      const auto value = msg.GetValue();
      const auto detail = msg.GetDetail();

      if (code == kCodeObject) {
        temp = msg.GetObj();
        auto tempId = detail + to_string(lambdaObjectCount); //detail start with "__"
        item.push_back(tempId); 
        lambdamap.insert(pair<string, Object>(tempId, temp));
        ++lambdaObjectCount;
      }
      else if (value == kStrRedirect && (code == kCodeSuccess || code == kCodeFillingSign)) {
        item.push_back(msg.GetDetail());
      }

      health = (value != kStrFatalError && value != kStrWarning);
      symbol.pop_back();
    }
    return health;
  }

  void Processor::EqualMark() {
    switch (symbol.empty()) {
    case true:
      symbol.push_back(currentToken); 
      break;
    case false:
      switch (defineLine) {
      case true:defineLine = false; break;
      case false:symbol.push_back(currentToken); break;
      }
      break;
    }
  }

  void Processor::Comma() {
    if (symbol.back() == kStrVar) {
      disableSetEntry = true;
    }
    if (disableSetEntry) {
      symbol.push_back(kStrVar);
      item.push_back(currentToken);
    }
  }

  bool Processor::LeftBracket(Message &msg) {
    auto result = true;
    if (symbol.back() == kStrVar) {
      msg.combo(kStrFatalError, kCodeIllegalCall, "Illegal pattern of definition.");
      result = false;
    }
    else {
      symbol.push_back(currentToken);
      item.push_back(currentToken);
    }
    return result;
  }

  bool Processor::RightBracket(Message &msg) {
    auto result = true;
    while (symbol.back() != "(" && !symbol.empty()) {
      result = Assemble(msg);
      if (!result) break;
    }

    if (result) {
      if (symbol.back() == "(") symbol.pop_back();
      result = Assemble(msg);
    }

    return result;
  }

  void Processor::LeftSquareBracket() {
    if (item.back().substr(0, 2) == "__") {
      operatorTargetType = lambdamap.find(item.back())->second.GetTypeId();
    }
    else {
      operatorTargetType = entry::FindObject(item.back())->GetTypeId();
    }
    item.push_back(currentToken);
    symbol.push_back(currentToken);
    subscriptProcessing = true;
  }

  bool Processor::RightSquareBracket(Message &msg) {
    bool result;
    deque<string> container;
    if (!subscriptProcessing) {
      //msg.combo
      result = false;
    }
    else {
      subscriptProcessing = false;
      while (symbol.back() != "[" && !symbol.empty()) {
        result = Assemble(msg);
        if (!result) break;
      }
      if (symbol.back() == "[") symbol.pop_back();
      while (item.back() != "[" && !item.empty()) {
        container.push_back(item.back());
        item.pop_back();
      }
      if (item.back() == "[") item.pop_back();
      if (!container.empty()) {
        switch (container.size()) {
        case 1:symbol.push_back("at:" + operatorTargetType + "|1"); break;
        case 2:symbol.push_back("at:" + operatorTargetType + "|2"); break;
        default:break;
        }

        while (!container.empty()) {
          item.push_back(container.back());
          container.pop_back();
        }

        result = Assemble(msg);
      }
      else {
        //msg.combo
        result = false;
      }
    }

    Kit().CleanupDeque(container);
    return result;
  }

  void Processor::Dot() {
    dotOperator = true;
  }

  void Processor::OtherSymbols() {
    if (symbol.empty()) {
      symbol.push_back(currentToken);
    }
    else if (GetPriority(currentToken) < GetPriority(symbol.back()) && symbol.back() != "(" && symbol.back() != "[") {
      auto j = symbol.size() - 1;
      auto k = item.size();
      while (symbol[j] != "(" && symbol.back() != "[" && GetPriority(currentToken) < GetPriority(symbol[j])) {
        if (k == item.size()) { k -= entry::GetRequiredCount(symbol[j]); }
        else { k -= entry::GetRequiredCount(symbol[j]) - 1; }
        --j;
      }
      symbol.insert(symbol.begin() + j + 1, currentToken);
      nextInsertSubscript = k;
      insertBtnSymbols = true;
    }
    else {
      symbol.push_back(currentToken);
    }
  }

  bool Processor::FunctionAndObject(Message &msg) {
    Kit kit;
    auto result = true, function = false;

    if (currentToken == kStrVar) defineLine = true;
    if (currentToken == kStrDef) functionLine = true;

    if (dotOperator) {
      const auto id = entry::GetTypeId(item.back());
      if (kit.FindInStringVector(currentToken, type::GetTemplate(id)->GetMethods())) {
        auto provider = entry::Order(currentToken, id);
        if (provider.Good()) {
          symbol.push_back(currentToken + ':' + id);
          function = true;
        }
        else {
          //TODO:???
        }
        
        dotOperator = false;
      }
      else {
        msg.combo(kStrFatalError, kCodeIllegalCall, "No such method/member in " + id + ".");
        result = false;
      }
    }
    else {
      switch (functionLine) {
      case true:
        item.push_back(currentToken);
        break;
      case false:
        switch (entry::Order(currentToken).Good()) {
        case true:symbol.push_back(currentToken); function = true; break;
        case false:item.push_back(currentToken); break;
        }
        break;
      }
    }

    if (!defineLine && function && nextToken != "(" && currentToken != kStrEnd) {
      errorString = "Bracket after function is missing.";
      this->health = false;
      result = false;
    }
    if (functionLine && forwardToken == kStrDef && nextToken != "(" && currentToken != kStrEnd) {
      errorString = "Illegal declaration of function.";
      this->health = false;
      result = false;
    }

    return result;
  }

  void Processor::OtherTokens() {
    switch (insertBtnSymbols) {
    case true:
      item.insert(item.begin() + nextInsertSubscript, currentToken);
      insertBtnSymbols = false;
      break;
    case false:
      item.push_back(currentToken);
      break;
    }
  }

  void Processor::FinalProcessing(Message &msg) {
    while (symbol.empty() != true) {
      if (symbol.back() == "(" || symbol.back() == ")") {
        msg.combo(kStrFatalError, kCodeIllegalSymbol, "Another bracket is expected.");
        break;
      }
      Assemble(msg);
    }
  }

  Message Processor::Start(size_t mode) {
    using namespace entry;
    Kit kit;
    Message result;
    auto state = true;
    const auto size = raw.size();

    if (health == false) {
      result.combo(kStrFatalError, kCodeBadExpression, errorString);
      return result;
    }

    this->mode = mode;
    lambdaObjectCount = 0;
    nextInsertSubscript = 0;
    operatorTargetType = kTypeIdNull;
    commaExpFunc = false,
    insertBtnSymbols = false,
    disableSetEntry = false,
    dotOperator = false,
    subscriptProcessing = false;
    functionLine = false;
    defineLine = false;

    for (size_t i = 0; i < size; ++i) {
      if (!health) break;
      if(!state) break;
      const auto unitType = types[i];
      currentToken = raw[i];
      if (i < size - 1) nextToken = raw[i + 1];
      else nextToken = kStrNull;
      result.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
      if (unitType == kTypeSymbol) {
        if (raw[i] == "=") EqualMark();
        else if (raw[i] == ",") Comma();
        else if (raw[i] == "[") LeftSquareBracket();
        else if (raw[i] == ".") Dot();
        else if (raw[i] == "(") state = LeftBracket(result);
        else if (raw[i] == "]") state = RightSquareBracket(result);
        else if (raw[i] == ")") state = RightBracket(result);
        else if (raw[i] == "++"); //
        else if (raw[i] == "--"); //
        else OtherSymbols();
      }
      else if (unitType == kGenericToken) state = FunctionAndObject(result);
      else if (unitType == kTypeNull) {
        result.combo(kStrFatalError, kCodeIllegalArgs, "Illegal token.");
        state = false;
      }
      else OtherTokens();
      forwardToken = currentToken;
    }

    if (state == true) FinalProcessing(result);

    kit.CleanupDeque(item).CleanupDeque(symbol);
    lambdamap.clear();

    if(!health) result.combo(kStrFatalError, kCodeBadExpression, errorString);

    return result;
  }

  Message EntryProvider::Start(ObjectMap &map) const {
    Message result;
    switch (this->Good()) {
    case true:result = activity(map); break;
    case false:result.combo(kStrFatalError, kCodeIllegalCall, "Illegal entry.");; break;
    }
    return result;
  }
}

