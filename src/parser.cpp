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
    vector<log_t> logger;

    void log(Message msg) { 
      time_t now = time(0);
#if defined(_WIN32)
      char nowtime[30] = { ' ' };
      ctime_s(nowtime, sizeof(nowtime), &now);
      logger.push_back(log_t(string(nowtime), msg));
#else
      string nowtime(ctime(&now));
      logger.push_back(log_t(nowtime, msg));
#endif
    }

    bool IsEmpty() {
      return logger.empty();
    }
  }

  namespace entry {
    EntryMap EntryMapBase;

    void Inject(string name, EntryProvider provider) {
      EntryMapBase.insert(EntryMapUnit(name, provider));
    }

    EntryProvider Order(string name) {
      EntryProvider result;
      EntryMap::iterator it = EntryMapBase.find(name);
      if (it != EntryMapBase.end()) {
        result = it->second;
      }
      return result;
    }

    EntryProvider Find(string target) {
      EntryProvider result;
      if (target == "+" || target == "-" || target == "*" || target == "/"
        || target == "==" || target == "<=" || target == ">=" || target == "!=") {
        result = Order("binexp");
      }
      else if (target == "=") {
        result = Order(kStrSetCmd);
      }
      else {
        EntryMap::iterator it = EntryMapBase.find(target);
        if (it != EntryMapBase.end()) {
          result = it->second;
        }
      }
      return result;
    }

    int GetRequiredCount(string target) {
      if (target == "+" || target == "-" || target == "*" || target == "/"
        || target == "==" || target == "<=" || target == ">=" || target == "!=") {
        return Order("binexp").GetArgumentSize();
      }
      EntryMap::iterator it = EntryMapBase.find(target);
      if (it != EntryMapBase.end()) {
        return it->second.GetArgumentSize();
      }
      return kCodeIllegalArgs;
    }

    void Delete(string name) {
      EntryMap::iterator it = EntryMapBase.find(name);
      if (it != EntryMapBase.end()) {
        EntryMapBase.erase(it);
      }
    }

    void ResetPluginEntry() {
      EntryMap tempbase;
      for (auto unit : EntryMapBase) {
        if (unit.second.GetPriority() != kFlagPluginEntry) tempbase.insert(unit);
      }
      EntryMapBase.swap(tempbase);
      tempbase.clear();
      EntryMap().swap(tempbase);
    }
  }

  int Kit::GetDataType(string target) {
    using std::regex_match;
    int result = kTypeNull;
    auto match = [&](const regex &pat) -> bool {
      return regex_match(target, pat);
    };

    if (target == kStrNull) result = kTypeNull;
    else if (target.front() == '"' && target.back() == '"') result = kTypeString;
    else if (match(kPatternBoolean)) result = kTypeBoolean;
    else if (match(kPatternFunction)) result = kTypeFunction;
    else if (match(kPatternInteger)) result = kTypeInteger;
    else if (match(kPatternDouble)) result = kTypeDouble;
    else if (match(kPatternSymbol)) result = kTypeSymbol;
    else if (match(kPatternBlank)) result = kTypeBlank;
    else result = kTypeNull;

    return result;
  }

  void Kit::PrintEvents() {
    using namespace trace;
    ofstream ofs("event.log", std::ios::trunc);
    string prioritystr;
    if (ofs.good()) {
      if (logger.empty()) {
        ofs << "No Events.\n";
      }
      else {
        for (log_t unit : logger) {
          ofs << "[" << unit.first << "]";
          if (unit.second.GetValue() == kStrFatalError) prioritystr = "Fatal:";
          else if (unit.second.GetValue() == kStrWarning) prioritystr = "Warning:";
          if (unit.second.GetDetail() != kStrEmpty) {
            ofs << prioritystr << unit.second.GetDetail() << "\n";
          }
        }
      }
    }
    ofs.close();
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
    vector<string> list = { kStrVar, "def", "return" };
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
    if (target == "=" || target == kStrVar) priority = 0;
    else if (target == "==" || target == ">=" || target == "<=") priority = 1;
    else if (target == "+" || target == "-") priority = 2;
    else if (target == "*" || target == "/" || target == "\\") priority = 3;
    else priority = 4;
    return priority;
  }

  Object Chainloader::GetObj(string name) {
    Object result;
    if (name.substr(0, 2) == "__") {
      map<string, Object>::iterator it = lambdamap.find(name);
      if (it != lambdamap.end()) result = it->second;
    }
    else {
      auto ptr = entry::FindObject(name);
      if (ptr != nullptr) result = *ptr;
    }
    return result;
  }

  bool Chainloader::Assemble(bool disable_set_entry, deque<string> &item, deque<string> &symbol,
    Message &msg, size_t mode) {
    Kit kit;
    AttrTag attrTag;
    EntryProvider provider = entry::Find(symbol.back());
    bool reversed = true, disable_query = false;
    size_t size = provider.GetArgumentSize();
    int arg_mode = provider.GetArgumentMode(), priority = provider.GetPriority();
    string id = provider.GetId();
    Object temp;
    deque<Object> objects;
      
    if (!provider.Good()) {
      msg.combo(kStrFatalError, kCodeIllegalCall, "Activity not found.");
      return false;
    }

    if (provider.GetId() == kStrDefineCmd) {
      disable_query = true;
    }

    if (priority == kFlagBinEntry) {
      size--;
      reversed = false;
    }
    if (disable_set_entry) {
      while (item.back() != "," && !item.empty()) {
        if (kit.GetDataType(item.back()) == kTypeFunction) {
          //query is completed in GetObj().
          if (disable_query) {
            attrTag.methods = type::GetTemplate(kTypeIdRawString)->GetMethods();
            attrTag.ro = true;
            temp.manage(item.back(), kTypeIdRawString, kit.MakeAttrTagStr(attrTag));
            disable_query = false;
          }
          else {
            temp = GetObj(item.back());
            if (temp.GetTypeId() != kTypeIdNull) {
              objects.push_back(temp);
            }
            else {
              //TODO:error
              break;
            }
          }
        }
        else {
          attrTag.methods = type::GetTemplate(kTypeIdRawString)->GetMethods();
          attrTag.ro = true;
          temp.manage(item.back(), kTypeIdRawString, kit.MakeAttrTagStr(attrTag));
        }
        item.pop_back();
      }
      if (!item.empty()) item.pop_back();
    }
    else {
      while (size > 0 && !item.empty()) {
        if (kit.GetDataType(item.back()) == kTypeFunction) {
          if (disable_query) {
            attrTag.methods = type::GetTemplate(kTypeIdRawString)->GetMethods();
            attrTag.ro = true;
            temp.manage(item.back(), kTypeIdRawString, kit.MakeAttrTagStr(attrTag));
            disable_query = false;
          }
          else {
            temp = GetObj(item.back());
          }
        }
        else {
          attrTag.methods = type::GetTemplate(kTypeIdRawString)->GetMethods();
          attrTag.ro = true;
          temp.manage(item.back(), kTypeIdRawString, kit.MakeAttrTagStr(attrTag));
        }
        if (temp.GetTypeId() != kTypeIdNull) {
          switch (reversed) {
          case true:objects.push_front(temp); break;
          case false:objects.push_back(temp); break;
          }
        }
        item.pop_back();
        size--;
      }
    }
    
    if (priority == kFlagBinEntry) {
      attrTag.methods = type::GetTemplate(kTypeIdRawString)->GetMethods();
      objects.push_back(Object().manage(symbol.back(), kTypeIdRawString, kit.MakeAttrTagStr(attrTag)));
    }

    switch (mode) {
    case kModeCycleJump:
      if (id == "end") {
        msg = provider.StartActivity(objects, this);
      }
      else if (id == "if") {
        msg.combo(kStrEmpty, kCodeFillingSign, kStrEmpty);
      }
      break;
    case kModeNextCondition:
      if (id == "if" || id == "elif" || id == "else" || id == "end") {
        msg = provider.StartActivity(objects, this);
      }
      else {
        msg.combo(kStrEmpty, kCodeSuccess, kStrEmpty);
      }
      break;
    case kModeNormal:
    default:
      msg = provider.StartActivity(objects, this);
    }

    if (msg.GetCode() == kCodeObject) {
      attrTag.methods = type::GetTemplate(msg.GetValue())->GetMethods();
      item.push_back(msg.GetDetail()); //detail start with "__"
      lambdamap.insert(pair<string, Object>(msg.GetDetail(),
        Object().set(msg.GetPtr(), msg.GetValue(), kit.MakeAttrTagStr(attrTag))));
    }
    else if (msg.GetValue() == kStrRedirect && msg.GetCode() == kCodeSuccess) {
      item.push_back(msg.GetDetail());
    }

    return (msg.GetValue() != kStrFatalError && msg.GetValue() != kStrWarning);
  }

  Message Chainloader::Start(size_t mode = kModeNormal) {
    using namespace entry;
    const size_t size = raw.size();
    size_t i = 0, j = 0, k = 0, next_ins_point = 0, forward_token_type = kTypeNull;
    string temp_str = kStrEmpty, temp_symbol = kStrEmpty, temp_sign = kStrEmpty;
    int unit_type = kTypeNull;
    bool comma_exp_func = false,
      string_token_proc = false, insert_btn_symbols = false,
      disable_set_entry = false, dot_operator = false;
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
          case false:if (symbol.back() != kStrVar) symbol.push_back(raw[i]); break;
          }
        }
        else if (raw[i] == ",") {
          if (symbol.back() == kStrVar) disable_set_entry = true;
          switch (disable_set_entry) {
          case true:
            symbol.push_back(kStrVar);
            item.push_back(raw[i]);
            break;
          case false:
            symbol.push_back(raw[i]);
            break;
          }
        }
        else if (raw[i] == "(") {
          symbol.push_back(raw[i]);
        }
        else if (raw[i] == "[") {
          if (kit.GetDataType(raw.at(i - 1)) == kTypeFunction) {
            symbol.push_back("__get_element");
          }
        }
        else if (raw[i] == ")" || raw[i] == "]") {
          if (raw[i] == ")") temp_symbol = "(";
          else if (raw[i] == "]") temp_symbol = "[";

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
        //TODO:dot operator processing
        //new
        if (dot_operator) {
          temp_str = entry::GetTypeId(item.back());
          if (temp_str != kTypeIdNull) {
            temp_sign = "__" + temp_str + "_" + raw.at(i);
            switch (entry::Find(temp_sign).Good()) {
            case true:symbol.push_back(temp_sign);break;
            case false:
              result.combo(kStrFatalError, kCodeIllegalCall, "No such method/member in " + temp_str + ".");
              fatal = true;
              continue;
              break;
            }
          }
        }
        else {
          switch (entry::Find(raw.at(i)).Good()) {
          case true:symbol.push_back(raw.at(i)); break;
          case false:item.push_back(raw.at(i)); break;
          }
        }
        dot_operator = false;
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

  Message EntryProvider::StartActivity(deque<Object> p, Chainloader *parent) {
    using namespace entry;
    ObjectMap map;
    Message result;
    AttrTag attrTag;
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

  vector<string> Kit::BuildStringVector(string source) {
    vector<string> result;
    string temp = kStrEmpty;
    for (auto unit : source) {
      if (unit == '|') {
        result.push_back(temp);
        temp.clear();
        continue;
      }
      temp.append(1, unit);
    }
    if (!temp.empty()) result.push_back(temp);
    return result;
  }

  AttrTag Kit::GetAttrTag(string target) {
    AttrTag result;
    char current = ' ', symbol = ' ';
    size_t size = target.size(), count = 0;
    string temp = kStrEmpty;

    for (count = 0; count < size; count++) {
      current = target.at(count);
      if (current == '+' || current == '%') {
        if (symbol != current) {
          switch (symbol) {
          case '+':
            result.methods.append(temp + "|");
            break;
          case '%':
            result.ro = (temp == kStrTrue);
            break;
          default:break;
          }
          temp = kStrEmpty;
        }
      }
      else {
        temp.append(1, current);
      }
    }

    if (result.methods.back() == '|') result.methods.pop_back();

    return result;
  }

  string Kit::MakeAttrTagStr(AttrTag target) {
    string result = kStrEmpty;
    vector<string> methods = this->BuildStringVector(target.methods);
    if (target.ro)result.append("%true");
    else result.append("%false");
    if (!methods.empty()) {
      for (auto &unit : methods) {
        result.append("+" + unit);
      }
    }
    return result;
  }

  //TODO:upgrade to new object system
  Message ChainStorage::Run(deque<string> res = deque<string>()) {
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

  Message Kit::ExecScriptFile(string target) {
    Message result;
    ScriptProvider sp(target.c_str());
    ChainStorage cs(sp);

    if (target == kStrEmpty) {
      trace::log(result.combo(kStrFatalError, kCodeIllegalArgs, "Empty path string."));
      return result;
    }
    Activiate();
    cs.Run();
    entry::ResetPlugin();
    //entry::CleanupObject();
    return result;
  }

  Message Quit(ObjectMap &p) {
    Message result(kStrEmpty, kCodeQuit, kStrEmpty);
    return result;
  }

  void Kit::Terminal() {
    using namespace entry;
    string buf = kStrEmpty;
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    Chainloader loader;
    auto Build = [&](string target) {return BuildStringVector(target); };
    std::cout << kEngineName << ' ' << kEngineVersion << std::endl;
    std::cout << kCopyright << ' ' << kEngineAuthor << std::endl;

    CreateManager();
    Activiate();
    Inject("quit", EntryProvider(ActivityTemplate().set("quit",Quit,kFlagNormalEntry,kCodeNormalArgs,"")));

    while (result.GetCode() != kCodeQuit) {
      std::cout << ">>>";
      std::getline(std::cin, buf);
      if (buf != kStrEmpty) {
        result = loader.Reset().Build(buf).Start();
        if (result.GetCode() < kCodeSuccess) {
          std::cout << result.GetDetail() << std::endl;
        }
      }
    }
    ResetPlugin();
    //CleanupObject();
  }
}

