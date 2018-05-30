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
        || id == "==" || id == "<=" || id == ">=" || id == "!=") {
        result = Order("binexp");
      }
      else if (id == "=") {
        result = Order(kStrSetCmd);
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
    using trace::log;
    string temp;
    current = 0;
    end = false;
    auto IsBlankStr = [](string target) -> bool {
      if (target == kStrEmpty || target.size() == 0) return true;
      for (const auto unit : target) {
        if (unit != ' ' || unit != '\n' || unit != '\t' || unit != '\r') {
          return false;
        }
      }
      return true;
    };

    stream.open(target, std::ios::in);
    if (stream.good()) {
      while (!stream.eof()) {
        std::getline(stream, temp);
        if (!IsBlankStr(temp)) {
          base.push_back(temp);
        }
      }
      if (!base.empty()) health = true;
      else health = false;
    }
    else {
      health = false;
    }
    stream.close();
  }

  Message ScriptProvider::Get() {
    Message result(kStrEmpty, kCodeSuccess, "");
    size_t size = base.size();

    if (current < size) {
      result.SetDetail(base.at(current));
      current++;
      if (current == size) {
        end = true;
      }
    }
    return result;
  }

  Chainloader &Chainloader::Build(string target) {
    using trace::log;
    Kit kit;
    vector<string> output;
    char bin_oper = NULL;
    size_t size = target.size();
    size_t i;
    string current = kStrEmpty;
    bool exempt_blank_char = false, string_processing = false;
    vector<string> list = { "var", "ref", "return" };
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

  int Chainloader::GetPriority(string target) const {
    int priority;
    if (target == "=" || target == "var") priority = 0;
    else if (target == "==" || target == ">=" || target == "<=") priority = 1;
    else if (target == "+" || target == "-") priority = 2;
    else if (target == "*" || target == "/" || target == "\\") priority = 3;
    else priority = 4;
    return priority;
  }

  Object *Chainloader::GetObj(string name) {
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

  vector<string> Chainloader::spilt(string target) {
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

  string Chainloader::GetHead(string target) {
    string result = kStrEmpty;
    for (auto &unit : target) {
      if (unit == ':') break;
      else result.append(1, unit);
    }
    return result;
  }

  bool Chainloader::Assemble(bool disable_set_entry, deque<string> &item, deque<string> &symbol,
    Message &msg, size_t mode) {
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

    if (id == kStrDefineCmd) { 
      if (item.size() == 1) {
        size = 1; //force reset to 1
      }
    }

    if (priority == kFlagOperatorEntry) { reversed = false; }
    if (!disable_set_entry) {
      while (size > 0 && !item.empty()) {
        switch (reversed) {
        case true:tokens.push_front(item.back()); break;
        case false:tokens.push_back(item.back()); break;
        }
        item.pop_back();
      }
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
    else {
      for (count = 0; count < size; count++) {
        if (tokens.size() - 1 < count) {
          map.insert(pair<string, Object>(GetName(args.at(count)), Object()));
        }
        else {
          token_type = kit.GetDataType(tokens.at(count));
          switch (token_type) {
          case kTypeFunction:
            if (args.at(count).front() == '&') {
              origin = GetObj(tokens.at(count));
              temp.ref(*origin);
              //temp.set(make_shared<Ref>(origin), kTypeIdRef, "%false");
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
        }
      }

      switch (priority) {
      case kFlagOperatorEntry:
        map.insert(pair<string, Object>(kStrOperator, Object()
          .manage(symbol.back(), kTypeIdRawString, kit.BuildAttrStr(strAttr))));
        break;
      case kFlagMethod:
        origin = GetObj(item.back());
        //map.insert(pair<string, Object>(kStrObject, Object()
        //  .set(make_shared<Ref>(origin), kTypeIdRef, "")));
        map.insert(pair<string, Object>(kStrObject, Object()
          .ref(*origin)));
      default:
        break;
      }
    }
    
    if (health) {
      switch (mode) {
      case kModeCycleJump:
        if (id == "end") msg = provider.Start(map);
        else if (id == "if") msg.combo(kStrEmpty, kCodeFillingSign, kStrEmpty);
        break;
      case kModeNextCondition:
        if (id == "if" || id == "elif" || id == "else" || id == "end") msg = provider.Start(map);
        else msg.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
        break;
      case kModeNormal:
      default:
        msg = provider.Start(map);
        break;
      }

      if (msg.GetCode() == kCodeObject) {
        attribute.methods = type::GetTemplate(msg.GetValue())->GetMethods();
        item.push_back(msg.GetDetail()); //detail start with "__"
        lambdamap.insert(pair<string, Object>(msg.GetDetail(),
          Object().set(msg.GetPtr(), msg.GetValue(), kit.BuildAttrStr(attribute))));
      }
      else if (msg.GetValue() == kStrRedirect && msg.GetCode() == kCodeSuccess) {
        item.push_back(msg.GetDetail());
      }

      health = (msg.GetValue() != kStrFatalError && msg.GetValue() != kStrWarning);
    }
    return health;
  }

  Message Chainloader::Start(size_t mode) {
    using namespace entry;
    const size_t size = raw.size();
    size_t i = 0, j = 0, k = 0, next_ins_point = 0, forward_token_type = kTypeNull;
    string temp_str = kStrEmpty, temp_symbol = kStrEmpty, temp_sign = kStrEmpty;
    string operator_target_type = kTypeIdNull;
    int unit_type = kTypeNull;
    bool comma_exp_func = false,
      string_token_proc = false, insert_btn_symbols = false,
      disable_set_entry = false, dot_operator = false,
      subscript_processing = false;
    bool fatal = false;
    Kit kit;
    Message result;

    deque<string> item, symbol;
    deque<string> container;

    for (i = 0; i < size; ++i) {
      if(fatal == true) break;
      unit_type = kit.GetDataType(raw.at(i));
      result.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
      if (unit_type == kTypeSymbol) {
        if (raw[i] == "\"") {
          switch (string_token_proc) {
          case true:item.back().append(raw[i]); break;
          case false:item.push_back(raw[i]); break;
          }
          string_token_proc = !string_token_proc;
        }
        else if (raw[i] == "=") {
          switch (symbol.empty()) {
          case true:symbol.push_back(raw[i]); break;
          case false:if (symbol.back() != "var") symbol.push_back(raw[i]); break;
          }
        }
        else if (raw[i] == ",") {
          if (symbol.back() == "var") disable_set_entry = true;
          switch (disable_set_entry) {
          case true:
            symbol.push_back("var");
            item.push_back(raw[i]);
            break;
          case false:
            break;
          }
        }
        else if (raw[i] == "(") {
          if (symbol.back() == kStrDefineCmd) {
            result.combo(kStrFatalError, kCodeIllegalCall, "Illegal pattern of definition.");
            fatal = true;
            continue;
          }
          else {
            symbol.push_back(raw[i]);
          }
        }
        else if (raw[i] == "[") {
          operator_target_type = entry::FindObject(item.back())->GetTypeId();
          item.push_back(raw.at(i));
          subscript_processing = true;
        }
        else if (raw[i] == "]") {
          container.clear();
          if (subscript_processing) {
            subscript_processing = false;
            while (item.back() != "[" && !item.empty()) {
              container.push_back(item.back());
              item.pop_back();
            }
            item.pop_back();
            if (!container.empty()) {
              switch (container.size()) {
              case 1:symbol.push_back("at:" + operator_target_type + "|2"); break;
              case 2:symbol.push_back("at:" + operator_target_type + "|3"); break;
              }
              while (!container.empty()) {
                item.push_back(container.back());
                container.pop_back();
              }
            }
          }
          else {
            fatal = true;
            continue;
          }
        }
        else if (raw[i] == ")") {
          if (raw[i] == ")") temp_symbol = "(";
          container.clear();
          while (symbol.back() != temp_symbol && symbol.empty() != true) {
            if (symbol.back() == ",") {
              container.push_back(item.back());
              item.pop_back();
              symbol.pop_back();
            }
            //Start point 1
            if (!Assemble(disable_set_entry, item, symbol, result, mode)) break;
            symbol.pop_back();
          }

          if (symbol.back() == temp_symbol) symbol.pop_back();
          while (!container.empty()) {
            item.push_back(container.back());
            container.pop_back();
          }
          //Start point 2
          if (!Assemble(disable_set_entry, item, symbol, result, mode)) break;
          symbol.pop_back();
        }
        else if (raw[i] == ".") {
          dot_operator = true;
        }
        else {
          if (symbol.empty()) {
            symbol.push_back(raw[i]);
          }
          else if (GetPriority(raw[i]) < GetPriority(symbol.back()) && symbol.back() != "(") {
            j = symbol.size() - 1;
            k = item.size();
            while (symbol.at(j) != "(" && GetPriority(raw[i]) < GetPriority(symbol.at(j))) {
              if (k = item.size()) { k -= entry::GetRequiredCount(symbol.at(j)); }
              else { k -= entry::GetRequiredCount(symbol.at(j)) - 1; }
              --j;
            }
            symbol.insert(symbol.begin() + j + 1, raw[i]);
            next_ins_point = k;
            insert_btn_symbols = true;

            j = 0;
            k = 0;
          }
          else {
            symbol.push_back(raw[i]);
          }
        }
      }
      else if (unit_type == kTypeFunction && !string_token_proc) {
        if (dot_operator) {
          temp_str = entry::GetTypeId(item.back());
          if (kit.FindInStringVector(raw.at(i), type::GetTemplate(temp_str)->GetMethods())) {
            auto provider = entry::Order(raw.at(i), temp_str);
            switch (provider.Good()) {
            case true:
              symbol.push_back(raw.at(i) + ':' + temp_str);
              break;
            case false:
              result.combo(kStrFatalError, kCodeIllegalCall, "No such method/member in " + temp_str + ".(01)");
              fatal = true;
              break;
            }
            continue;
          }
          else {
            result.combo(kStrFatalError, kCodeIllegalCall, "No such method/member in " + temp_str + ".(02)");
            fatal = true;
            continue;
          }
        }
        else {
          switch (entry::Order(raw.at(i)).Good()) {
          case true:symbol.push_back(raw.at(i)); break;
          case false:item.push_back(raw.at(i)); break;
          }
        }
        dot_operator = false;
      }
      else if (unit_type == kTypeNull) {
        fatal = true;
      }
      else {
        switch (insert_btn_symbols) {
        case true:
          item.insert(item.begin() + next_ins_point, raw[i]);
          insert_btn_symbols = false;
          break;
        case false:
          switch (string_token_proc) {
          case true:item.back().append(raw[i]); break;
          case false:item.push_back(raw[i]); break;
          }
          break;
        }
      }
    }

    if (!fatal) {
      while (symbol.empty() != true) {
        if (symbol.back() == "(" || symbol.back() == ")") {
          result.combo(kStrFatalError, kCodeIllegalSymbol, "Another bracket expected.");
          break;
        }
        //Start point 3
        if (!Assemble(disable_set_entry, item, symbol, result, mode)) break;
        symbol.pop_back();
      }
    }

    kit.CleanupDeque(container).CleanupDeque(item).CleanupDeque(symbol);

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

  Message EntryProvider::StartActivity(deque<Object> p, Chainloader *parent) {
    using namespace entry;
    ObjectMap map;
    Message result;
    Attribute attribute;
    bool ignore_first_arg = true, health = true;
    size_t size = p.size(), expected = this->args.size(), i = 0;
    
    if (arg_mode == kCodeAutoFill) {
      if (size > expected) {
        health = false;
        result.combo(kStrFatalError, kCodeIllegalArgs, "Parameter doesn't match expected count.(01)");
      }
      else {
        for (i = 0; i < expected; i++) {
          if (i > size - 1) map.insert(pair<string, Object>(args.at(i), Object()));
          else map.insert(pair<string, Object>(args.at(i), p.at(i)));
        }
      }
    }
    else if (arg_mode == kCodeNormalArgs) {
      if (size != expected) {
        health = false;
        result.combo(kStrFatalError, kCodeIllegalArgs, "Parameter count doesn't match expected count.(02)");
      }
      else {
        for (i = 0; i < size; i++) {
          map.insert(pair<string, Object>(args.at(i), p.at(i)));
        }
      }
    }

    if (health) {
      result = activity(map);
    }

    return result;
  }

  Message ChainStorage::Run(deque<string> res) {
    using namespace entry;
    Message result;
    size_t i = 0;
    size_t size = 0;
    size_t tail = 0;
    stack<size_t> nest;
    stack<bool> state_stack;
    stack<size_t> mode_stack;
    bool already_executed = false;
    size_t current_mode = kModeNormal;

    CreateManager();
    //TODO:add custom function support
    if (!res.empty()) {
      if (res.size() != parameter.size()) {
        result.combo(kStrFatalError, kCodeIllegalCall, "wrong parameter count.");
        return result;
      }
      for (i = 0; i < parameter.size(); i++) {
        //CreateObject(parameter.at(i), res.at(i), false);
      }
    }

    if (!storage.empty()) {
      size = storage.size();
      i = 0;
      while (i < size) {
        result = storage.at(i).Start(current_mode);

        //error
        if (result.GetValue() == kStrFatalError) break;

        //return
        if (result.GetCode() == kCodeReturn) {
          result.SetCode(kCodeSuccess);
          break;
        }

        //condition
        if (result.GetCode() == kCodeConditionRoot) {
          state_stack.push(already_executed);
          mode_stack.push(current_mode);

          if (result.GetValue() == kStrFalse) {
            current_mode = kModeNextCondition;
            already_executed = false;
          }
          else {
            already_executed = true;
          }
        }
        if (result.GetCode() == kCodeConditionBranch) {
          if (!already_executed && current_mode == kModeNextCondition && result.GetValue() == kStrTrue) {
            current_mode = kModeNormal;
            already_executed = true;
          }
          else if (already_executed && current_mode == kModeNormal) {
            current_mode = kModeNextCondition;
          }
        }
        if (result.GetCode() == kCodeConditionLeaf) {
          if (!already_executed && current_mode == kModeNextCondition) {
            current_mode = kModeNormal;
          }
          else if (already_executed && current_mode == kModeNormal) {
            current_mode = kModeNextCondition;
          }
        }
        if (result.GetCode() == kCodeFillingSign) {
          mode_stack.push(kModeCycleJump);
        }

        //loop
        if (result.GetCode() == kCodeHeadSign && result.GetValue() == kStrTrue) {
          current_mode = kModeCycle;
          if (nest.empty()) nest.push(i);
          else if (nest.top() != i) nest.push(i);
        }
        if (result.GetCode() == kCodeHeadSign && result.GetValue() == kStrFalse) {
          if (nest.empty()) {
            current_mode = kModeCycleJump;
          }
          else {
            current_mode = kModeCycle;
            nest.pop();
            i = tail;
          }

        }

        //tail
        if (result.GetCode() == kCodeTailSign) {
          if (current_mode == kModeCycle) {
            if (!nest.empty()) {
              tail = i;
              i = nest.top();
              continue;
            }
          }
          else if (current_mode = kModeNextCondition) {
            if (!state_stack.empty()) {
              already_executed = state_stack.top();
              state_stack.pop();
            }
          }
          else if (current_mode == kModeCycleJump) {
            if (mode_stack.empty()) {
              current_mode = kModeNormal;
            }
            else if (mode_stack.top() == kModeCycleJump) {
              mode_stack.pop();
            }
            else {
              current_mode = kModeNormal;
            }
          }
        }

        i++;
      }
    }

    DisposeManager();
    return result;
  }


}

