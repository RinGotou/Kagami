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
#include <iostream>
#include <ctime>
#include "parser.h"

namespace kagami {
  namespace trace {
    vector<log_t> &GetLogger() {
      static vector<log_t> base;
      return base;
    }

    void log(Message msg) { 
      time_t now = time(0);
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
      size_t argsize = 0;

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

    int GetRequiredCount(string target) {
      EntryProvider provider;
      int result;
      if (target == "+" || target == "-" || target == "*" || target == "/"
        || target == "==" || target == "<=" || target == ">=" || target == "!=") {
        result = Order("binexp").GetArgumentSize();
      }
      else {
        provider = Order(target);
        switch (provider.Good()) {
        case true:result = provider.GetArgumentSize(); break;
        case false:result = kCodeIllegalArgs; break;
        }
      }
      return result;
    }

    void RemoveByTemplate(ActivityTemplate temp) {
      vector<EntryProvider> &base = GetEntryBase();
      vector<EntryProvider>::iterator it;

      for (it = base.begin(); it != base.end(); it++) {
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
        if (!IsBlankStr(temp)) {
          storage.push_back(Processor().Reset().Build(temp));
        }
      }
      if (!storage.empty()) health = true;
    }
    stream.close();
  }

  Message ScriptProvider::Run(deque<string> res) {
    //using namespace entry;
    Message result;
    size_t count;
    stack<size_t> cycleNestStack;
    stack<size_t> cycleTailStack;
    stack<bool> conditionStack;
    stack<size_t> modeStack;
    size_t currentMode = kModeNormal;
    size_t nestHeadCount = 0;
    int code = kCodeSuccess;
    string value = kStrEmpty;
    bool health = true;
    bool modeStackLock = false;

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
      for (count = 0; count < parameters.size(); count++) {
        //CreateObject(parameter.at(i), res.at(i), false);
      }
    }

    //Main state machine
    while (current < storage.size()) {
      if (!health) break;
      result = storage.at(current).Start(currentMode);
      value = result.GetValue();
      code = result.GetCode();

      if (value == kStrFatalError) break;
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
            if (cycleTailStack.empty()) cycleTailStack.push(current - 1);
            current = cycleNestStack.top();
            break;
          case kModeCycleJump:
            currentMode = modeStack.top();
            modeStack.pop();
            cycleNestStack.pop();
            cycleTailStack.pop();
            break;
          }
        }
      }
      ++current;
    }

    entry::DisposeManager();

    return result;
  }

  Processor &Processor::Build(string target) {
    using trace::log;
    Kit kit;
    vector<string> output;
    char bin_oper = NULL;
    size_t size = target.size();
    size_t i;
    string current = kStrEmpty;
    bool exempt_blank_char = false, string_processing = false;
    vector<string> list = { "var", "def", "return" };
    auto ToString = [](char c) -> string {
      return string().append(1, c);
    };

    for (i = 0; i < size; i++) {
      if (!exempt_blank_char) {
        if (kit.GetDataType(ToString(target[i])) == kTypeBlank) {
          continue;
        }
        else if (kit.GetDataType(ToString(target[i])) != kTypeBlank) {
          exempt_blank_char = true;
        }
      }

      if (target[i] == '"') {
        if (string_processing && target[i - 1] != '\\' && i - 1 >= 0) {
          string_processing = !string_processing;
          current.append(1, target.at(i));
          output.push_back(current);
          current = kStrEmpty;
          continue;
        }
        else if (!string_processing) {
          string_processing = !string_processing;
        }
      }

      switch (target[i]) {
      case '(':
      case ',':
      case ')':
      case '[':
      case ']':
      case '{':
      case '}':
      case ':':
      case '+':
      case '-':
      case '*':
      case '/':
      case '.':
        if (string_processing) {
          current.append(1, target[i]);
        }
        else {
          if (current != kStrEmpty) output.push_back(current);
          output.push_back(ToString(target[i]));
          current = kStrEmpty;
        }
        break;
      case '"':
        if (string_processing) {
          current.append(1, target.at(i));
        }
        else if (i > 0) {
          if (target.at(i - 1) == '\\') {
            current.append(1, target.at(i));
          }
        }
        else {
          if (current != kStrEmpty) output.push_back(current);
          output.push_back(ToString(target[i]));
          current = kStrEmpty;
        }
        break;
      case '=':
      case '>':
      case '<':
      case '!':
        if (string_processing) {
          current.append(1, target[i]);
        }
        else {
          if (i + 1 < size && target[i + 1] == '=') {
            bin_oper = target[i];
            if (current != kStrEmpty) output.push_back(current);
            current = kStrEmpty;
            continue;
          }
          else if (bin_oper != NULL) {
            string binaryopt = { bin_oper, target[i] };
            if (kit.GetDataType(binaryopt) == kTypeSymbol) {
              output.push_back(binaryopt);
              bin_oper = NULL;
            }
          }
          else {
            if (current != kStrEmpty) output.push_back(current);
            output.push_back(ToString(target[i]));
            current = kStrEmpty;
          }
        }
        break;
      case ' ':
      case '\t':
        if (string_processing) {
          current.append(1, target[i]);
        }
        else if (kit.Compare(current, list) && output.empty()) {
          if (i + 1 < size && target[i + 1] != ' ' && target[i + 1] != '\t') {
            output.push_back(current);
            current = kStrEmpty;
          }
          continue;
        }
        else {
          if ((std::regex_match(ToString(target[i + 1]), kPatternSymbol)
            || std::regex_match(ToString(target[i - 1]), kPatternSymbol)
            || target[i - 1] == ' ' || target[i - 1] == '\t')
            && i + 1 < size) {
            continue;
          }
          else {
            continue;
          }
        }
        break;
      case '#':
        if (!string_processing) break;
      default:
        current.append(1, target[i]);
        break;
      }
    }

    if (current != kStrEmpty) output.push_back(current);
    raw = output;
    kit.CleanupVector(output);

    return *this;
  }

  int Processor::GetPriority(string target) const {
    int priority;
    if (target == "=" || target == "var") priority = 0;
    else if (target == "==" || target == ">=" || target == "<=") priority = 1;
    else if (target == "+" || target == "-") priority = 2;
    else if (target == "*" || target == "/" || target == "\\") priority = 3;
    else priority = 4;
    return priority;
  }

  Object *Processor::GetObj(string name) {
    Object *result = new Object();
    if (name.substr(0, 2) == "__") {
      map<string, Object>::iterator it = lambdamap.find(name);
      if (it != lambdamap.end()) result = &(it->second);
    }
    else {
      auto ptr = entry::FindObject(name);
      if (ptr != nullptr) result = ptr;
    }
    return result;
  }

  vector<string> Processor::spilt(string target) {
    vector<string> result;
    string temp = kStrEmpty;
    bool start = false;
    for (auto &unit : target) {
      if (unit == ':') {
        start = true;
        continue;
      }
      if (start) {
        temp.append(1, unit);
      }
    }
    result = Kit().BuildStringVector(temp);
    return result;
  }

  string Processor::GetHead(string target) {
    string result = kStrEmpty;
    for (auto &unit : target) {
      if (unit == ':') break;
      else result.append(1, unit);
    }
    return result;
  }

  bool Processor::Assemble(Message &msg) {
    Kit kit;
    Attribute strAttr(type::GetTemplate(kTypeIdRawString)->GetMethods(), false);
    Attribute attribute;
    EntryProvider provider;
    Object temp, *origin;
    size_t size = 0, count = 0;
    int provider_size = -1, arg_mode = 0, priority = 0, token_type = kTypeNull;
    bool reversed = true, disable_query = false, method = false, health = true;
    string id = GetHead(symbol.back()), provider_type = kTypeIdNull;
    vector<string> tags = spilt(symbol.back());
    vector<string> args;
    deque<string> tokens;
    ObjectMap map;

    auto GetName = [](string target) ->string {
      if (target.front() == '&' || target.front() == '%')
        return target.substr(1, target.size() - 1);
      else return target;
    };

    if (!tags.empty()) {
      provider_type = tags.front();
      if (tags.size() > 1) {
        provider_size = stoi(tags.at(1));
      }
      else {
        provider_size = -1;
      }
    }

    provider = entry::Order(id, provider_type, provider_size);

    if (!provider.Good()) {
      msg.combo(kStrFatalError, kCodeIllegalCall, "Activity not found.");
      return false;
    }
    else {
      size = provider.GetArgumentSize();
      arg_mode = provider.GetArgumentMode();
      priority = provider.GetPriority();
      args = provider.GetArguments();
    }

    if (priority == kFlagOperatorEntry) { 
      reversed = false; 
    }
    if (!disable_set_entry) {
      count = size;
      while (count > 0 && !item.empty() && item.back() != "(") {
        switch (reversed) {
        case true:tokens.push_front(item.back()); break;
        case false:tokens.push_back(item.back()); break;
        }
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

    if (arg_mode == kCodeNormalArgs && (tokens.size() < size || tokens.size() > size)) {
      msg.combo(kStrFatalError, kCodeIllegalArgs, "Parameter doesn't match expected count.(01)");
      health = false;
    }
    else if (arg_mode == kCodeAutoFill && tokens.size() < 1) {
      msg.combo(kStrFatalError, kCodeIllegalArgs, "You should provide at least one parameter.(02)");
      health = false;
    }
    else {
      for (count = 0; count < size; count++) {
        if (tokens.size() - 1 < count) {
          map.insert(pair<string, Object>(GetName(args.at(count)), Object()));
        }
        else {
          token_type = kit.GetDataType(tokens.at(count));
          switch (token_type) {
          case kGenericToken:
            if (args.at(count).front() == '&') {
              origin = GetObj(tokens.at(count));
              temp.ref(*origin);
            }
            else if (args.at(count).front() == '%') {
              temp.manage(tokens.at(count), kTypeIdRawString, kit.BuildAttrStr(strAttr));
            }
            else {
              temp = *GetObj(tokens.at(count));
            }
            break;
          default:
            temp.manage(tokens.at(count), kTypeIdRawString, kit.BuildAttrStr(strAttr));
            break;
          }
          map.insert(pair<string,Object>(GetName(args.at(count)), temp));
          temp.clear();
        }
      }

      switch (priority) {
      case kFlagOperatorEntry:
        map.insert(pair<string, Object>(kStrOperator, Object()
          .manage(symbol.back(), kTypeIdRawString, kit.BuildAttrStr(strAttr))));
        break;
      case kFlagMethod:
        origin = GetObj(item.back());
        map.insert(pair<string, Object>(kStrObject, Object()
          .ref(*origin)));
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

      if (msg.GetCode() == kCodeObject) {
        temp = msg.GetObj();
        auto tempId = msg.GetDetail() + to_string(lambdaObjectCount); //detail start with "__"
        item.push_back(tempId); 
        lambdamap.insert(pair<string, Object>(tempId, temp));
        ++lambdaObjectCount;
      }
      else if (msg.GetValue() == kStrRedirect && (msg.GetCode() == kCodeSuccess || msg.GetCode() == kCodeFillingSign)) {
        item.push_back(msg.GetDetail());
      }

      health = (msg.GetValue() != kStrFatalError && msg.GetValue() != kStrWarning);
      symbol.pop_back();
    }
    return health;
  }

  void Processor::DoubleQuotationMark() {
    switch (string_token_proc) {
    case true:item.back().append(currentToken); break;
    case false:item.push_back(currentToken); break;
    }
    string_token_proc = !string_token_proc;
  }

  void Processor::EqualMark() {
    switch (symbol.empty()) {
    case true:
      symbol.push_back(currentToken); 
      break;
    case false:
      if (symbol.back() != kStrVar) symbol.push_back(currentToken);
      //else item.push_back(currentToken);
      break;
    }
  }

  void Processor::Comma() {
    if (symbol.back() == kStrVar) {
      disable_set_entry = true;
    }
    if (disable_set_entry) {
      symbol.push_back(kStrVar);
      item.push_back(currentToken);
    }
  }

  bool Processor::LeftBracket(Message &msg) {
    bool result = true;
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
    bool result = true;
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
    operatorTargetType = entry::FindObject(item.back())->GetTypeId();
    item.push_back(currentToken);
    subscript_processing = true;
  }

  bool Processor::RightSquareBracket(Message &msg) {
    bool result = true;
    deque<string> container;
    if (!subscript_processing) {
      //msg.combo
      result = false;
    }
    else {
      subscript_processing = false;
      while (item.back() != "[" && !item.empty()) {
        container.push_back(item.back());
        item.pop_back();
      }
      if (item.back() == "[") item.pop_back();
      if (!container.empty()) {
        switch (container.size()) {
        case 1:symbol.push_back("at:" + operatorTargetType + "|1"); break;
        case 2:symbol.push_back("at:" + operatorTargetType + "|2"); break;
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
    dot_operator = true;
  }

  void Processor::OtherSymbols() {
    size_t j, k;
    if (symbol.empty()) {
      symbol.push_back(currentToken);
    }
    else if (GetPriority(currentToken) < GetPriority(symbol.back()) && symbol.back() != "(") {
      j = symbol.size() - 1;
      k = item.size();
      while (symbol.at(j) != "(" && GetPriority(currentToken) < GetPriority(symbol.at(j))) {
        if (k = item.size()) { k -= entry::GetRequiredCount(symbol.at(j)); }
        else { k -= entry::GetRequiredCount(symbol.at(j)) - 1; }
        --j;
      }
      symbol.insert(symbol.begin() + j + 1, currentToken);
      nextInsertSubscript = k;
      insert_btn_symbols = true;

      j = 0;
      k = 0;
    }
    else {
      symbol.push_back(currentToken);
    }
  }

  bool Processor::FunctionAndObject(Message &msg) {
    Kit kit;
    string id = kStrEmpty;
    bool result = true;

    if (dot_operator) {
      id = entry::GetTypeId(item.back());
      if (kit.FindInStringVector(currentToken, type::GetTemplate(id)->GetMethods())) {
        symbol.push_back(currentToken + ':' + id);
        dot_operator = false;
      }
      else {
        msg.combo(kStrFatalError, kCodeIllegalCall, "No such method/member in " + id + ".(02)");
        result = false;
      }
    }
    else {
      switch (entry::Order(currentToken).Good()) {
      case true:symbol.push_back(currentToken); break;
      case false:item.push_back(currentToken); break;
      }
    }
    return result;
  }

  void Processor::OtherTokens() {
    switch (insert_btn_symbols) {
    case true:
      item.insert(item.begin() + nextInsertSubscript, currentToken);
      insert_btn_symbols = false;
      break;
    case false:
      switch (string_token_proc) {
      case true:item.back().append(currentToken); break;
      case false:item.push_back(currentToken); break;
      }
      break;
    }
  }

  void Processor::FinalProcessing(Message &msg) {
    while (symbol.empty() != true) {
      if (symbol.back() == "(" || symbol.back() == ")") {
        msg.combo(kStrFatalError, kCodeIllegalSymbol, "Another bracket expected.");
        break;
      }
      Assemble(msg);
    }
  }

  Message Processor::Start(size_t mode) {
    using namespace entry;
    const size_t size = raw.size();
    size_t i = 0;
    int unit_type = kTypeNull;
    Kit kit;
    Message result;
    bool health = true;

    lambdaObjectCount = 0;
    nextInsertSubscript = 0;
    this->mode = mode;
    operatorTargetType = kTypeIdNull;
    comma_exp_func = false,
    string_token_proc = false,
    insert_btn_symbols = false,
    disable_set_entry = false,
    dot_operator = false,
    subscript_processing = false;

    for (i = 0; i < size; ++i) {
      if(!health) break;
      currentToken = raw.at(i);
      unit_type = kit.GetDataType(raw.at(i));
      result.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
      if (unit_type == kTypeSymbol) {
        if (raw[i] == "\"") DoubleQuotationMark();
        else if (raw[i] == "=") EqualMark();
        else if (raw[i] == ",") Comma();
        else if (raw[i] == "[") LeftSquareBracket();
        else if (raw[i] == ".") Dot();
        else if (raw[i] == "(") health = LeftBracket(result);
        else if (raw[i] == "]") health = RightSquareBracket(result);
        else if (raw[i] == ")") health = RightBracket(result);
        else OtherSymbols();
      }
      else if (unit_type == kGenericToken && !string_token_proc) health = FunctionAndObject(result);
      else if (unit_type == kTypeNull) {
        result.combo(kStrFatalError, kCodeIllegalArgs, "Illegal token.");
        health = false;
      }
      else OtherTokens();
    }

    if (health) FinalProcessing(result);

    kit.CleanupDeque(item).CleanupDeque(symbol);
    lambdamap.clear();


    return result;
  }

  Message EntryProvider::Start(ObjectMap &map) {
    Message result;
    switch (this->Good()) {
    case true:result = activity(map); break;
    case false:result.combo(kStrFatalError, kCodeIllegalCall, "Illegal entry.");; break;
    }
    return result;
  }
}

