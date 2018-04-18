//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Kagami Nakamura
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
#include "parser.h"

namespace Tracking {
  vector<Message> base;

  void log(Message msg) {
    base.push_back(msg);
  }

  bool IsEmpty() {
    return base.empty();
  }
}

namespace Entry {
  EntryMap EntryMapBase;

  void Inject(string name, EntryProvider provider) {
    EntryMapBase.insert(EntryMapUnit(name, provider));
  }

  Message FindAndExec(string name, deque<string> res) {
    Message result(kStrFatalError, kCodeIllegalCall, "Entry is not found.");
    EntryMap::iterator it = EntryMapBase.find(name);
    if (it != EntryMapBase.end()) {
      result = it->second.StartActivity(res, nullptr);
    }
    return result;
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
      || target == "==" || target == "<=" || target == ">=") {
      result = Order("binexp");
    }
    else if (target == "=") {
      result = Order("set");
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
      || target == "==" || target == "<=" || target == ">=") {
      return Order("binexp").GetRequiredCount() - 1;
    }
    EntryMap::iterator it = EntryMapBase.find(target);
    if (it != EntryMapBase.end()) {
      return it->second.GetRequiredCount();
    }
    return kFlagNotDefined;
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

namespace Kagami {
  int Kit::GetDataType(string target) {
    using std::regex_match;
    int result = kTypeNull;
    auto match = [&](const regex &pat) -> bool {
      return regex_match(target, pat);
    };

    if (target == kStrNull) result = kTypeNull;
    else if (match(kPatternFunction)) result = kTypeFunction;
    else if (match(kPatternBoolean)) result = kTypeBoolean;
    else if (match(kPatternInteger)) result = kTypeInteger;
    else if (match(kPatternDouble)) result = KTypeDouble;
    else if (match(kPatternSymbol)) result = kTypeSymbol;
    else if (match(kPatternBlank)) result = kTypeBlank;
    else if (target.front() == '"' && target.back() == '"') result = kTypeString; //temporary fix for string ploblem
    else result = kTypeNull;

    return result;
  }

  void Kit::PrintEvents() {
    using namespace Tracking;
    ofstream ofs("event.log", std::ios::trunc);
    string prioritystr;
    if (ofs.good()) {
      if (base.empty()) {
        ofs << "No Events.\n";
      }
      else {
        for (auto unit : base) {
          if (unit.GetValue() == kStrFatalError) prioritystr = "Fatal:";
          else if (unit.GetValue() == kStrWarning) prioritystr = "Warning:";
          if (unit.GetDetail() != kStrEmpty) {
            ofs << prioritystr << unit.GetDetail() << "\n";
          }
        }
      }
    }
    ofs.close();
  }

  ScriptProvider::ScriptProvider(const char *target) {
    using Tracking::log;
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

  bool Chainloader::StartActivity(EntryProvider &provider, deque<string> container, deque<string> &item,
    size_t top, Message &msg, Chainloader *loader) {
    bool rv = true;
    int code = kCodeSuccess;
    string value = kStrEmpty;
    Message temp;

    if (provider.Good()) {
      temp = provider.StartActivity(container, loader);
      code = temp.GetCode();
      value = temp.GetValue();
      if (code < kCodeSuccess) Tracking::log(temp);
      msg = temp;
    }
    else {
      Tracking::log(msg.combo(kStrFatalError, kCodeIllegalCall, "Activity not found"));
      rv = false;
    }

    return rv;
  }

  Chainloader &Chainloader::Build(string target) {
    using Tracking::log;
    Kit Kit;
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
#if defined(_DEBUG_FLAG_)
    if (target == kStrEmpty) {
      log(Message(kStrWarning, kCodeIllegalArgs, "Chainloader::Build() 1"));
      return *this;
    }
#endif
    for (i = 0; i < size; i++) {
      if (!exempt_blank_char) {
        if (Kit.GetDataType(ToString(target[i])) == kTypeBlank) {
          continue;
        }
        else if (Kit.GetDataType(ToString(target[i])) != kTypeBlank) {
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
      case '{':
      case '}':
      case ':':
      case '+':
      case '-':
      case '*':
      case '/':
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
            if (Kit.GetDataType(binaryopt) == kTypeSymbol) {
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
        else if (Kit.Compare(current, list) && output.empty()) {
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
    Kit.CleanupVector(output);

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

  bool Chainloader::ShuntingYardProcessing(bool disableset, deque<string> &item, deque<string> &symbol,
    Message &msg, bool next_condition = false) {
    using namespace Entry;
    EntryProvider provider = Find(symbol.back());
    bool result = false, reversed = true;
    int count = provider.GetRequiredCount();
    deque<string> container;

    if (provider.GetPriority() == kFlagBinEntry) {
      if (count != kFlagAutoSize) count -= 1;
      reversed = false;
    }
    if (disableset == true) {
      while (item.back() != "," && item.empty() == false) {
        container.push_back(item.back());
        item.pop_back();
      }
      if (!item.empty()) item.pop_back(); //pop ","
    }
    else {
      while (count != 0 && item.empty() != true) {
        switch (reversed) {
        case true:container.push_front(item.back()); break;
        case false:container.push_back(item.back()); break;
        }
        item.pop_back();
      }
      count--;
    }
    if (provider.GetPriority() == kFlagBinEntry) container.push_back(symbol.back());
    result = StartActivity(provider, container, item, item.size(), msg, this);
    if (msg.GetCode() == kCodePoint) {
      item.push_back(msg.GetDetail());
      PointWrapper wrapper;
      wrapper.set(msg.GetCastPath(), msg.GetValue());
      lambdamap.insert(pair<string, PointWrapper>(msg.GetDetail(), wrapper));
    }
    else if (msg.GetValue() == kStrRedirect && msg.GetCode() == kCodeSuccess) {
      item.push_back(msg.GetDetail());
    }
    return result;
  }

  Message Chainloader::Start(int mode = kModeNormal) {
    using namespace Entry;
    const size_t size = raw.size();
    Kit Kit;
    Message result;
    size_t i = 0, j = 0, k = 0, nextinspoint = 0, forwardtype = kTypeNull;
    string tempstr = kStrEmpty;
    int unitType = kTypeNull;
    bool commaexp = false, directappend = false, forwardinsert = false, disableset = false, next_condition = false;

    deque<string> item, symbol;
    deque<string> container;

    if (mode == kModeNextCondition) next_condition = true;

    for (i = 0; i < size; ++i) {
      unitType = Kit.GetDataType(raw.at(i));
      if (unitType == kTypeSymbol) {
        if (raw[i] == "\"") {
          switch (directappend) {
          case true:item.back().append(raw[i]); break;
          case false:item.push_back(raw[i]); break;
          }
          directappend = !directappend;
        }
        else if (raw[i] == "=") {
          switch (symbol.empty()) {
          case true:symbol.push_back(raw[i]); break;
          case false:if (symbol.back() != kStrVar) symbol.push_back(raw[i]); break;
          }
        }
        else if (raw[i] == ",") {
          if (symbol.back() == kStrVar) disableset = true;
          switch (disableset) {
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
          if (symbol.empty() || Kit.GetDataType(symbol.back()) == kTypeSymbol) {
            symbol.push_back("commaexp");
          }
          symbol.push_back(raw[i]);
        }
        else if (raw[i] == ")") {
          while (symbol.back() != "(" && symbol.empty() != true) {
            if (symbol.back() == ",") {
              container.push_back(item.back());
              item.pop_back();
              symbol.pop_back();
            }
            //Start point 1
            if (!ShuntingYardProcessing(disableset, item, symbol, result,next_condition)) break;
            symbol.pop_back();
          }

          if (symbol.back() == "(") symbol.pop_back();
          while (!container.empty()) {
            item.push_back(container.back());
            container.pop_back();
          }
          //Start point 2
          if (!ShuntingYardProcessing(disableset, item, symbol, result, next_condition)) break;
          symbol.pop_back();
        }
        else {
          if (symbol.empty()) {
            symbol.push_back(raw[i]);
          }
          else if (GetPriority(raw[i]) < GetPriority(symbol.back()) && symbol.back() != "(") {
            j = symbol.size() - 1;
            k = item.size();
            while (symbol.at(j) != "(" && GetPriority(raw[i]) < GetPriority(symbol.at(j))) {
              if (k = item.size()) { k -= Entry::GetRequiredCount(symbol.at(j)); }
              else { k -= Entry::GetRequiredCount(symbol.at(j)) - 1; }
              --j;
            }
            symbol.insert(symbol.begin() + j + 1, raw[i]);
            nextinspoint = k;
            forwardinsert = true;

            j = 0;
            k = 0;
          }
          else {
            symbol.push_back(raw[i]);
          }
        }
      }
      else if (unitType == kTypeFunction && !directappend) {
        switch (Find(raw[i]).Good()) {
        case true:
          symbol.push_back(raw[i]);
          break;
        case false:
          item.push_back(raw[i]);
          break;
        }
      }
      else {
        switch (forwardinsert) {
        case true:
          item.insert(item.begin() + nextinspoint, raw[i]);
          forwardinsert = false;
          break;
        case false:
          switch (directappend) {
          case true:item.back().append(raw[i]); break;
          case false:item.push_back(raw[i]); break;
          }
          break;
        }
      }
    }

    if (result.GetValue() != kStrFatalError) {
      while (symbol.empty() != true) {
        if (symbol.back() == "(" || symbol.back() == ")") {
          result.combo(kStrFatalError, kCodeIllegalSymbol, "Another bracket expected.");
          break;
        }
        //Start point 3
        if (!ShuntingYardProcessing(disableset, item, symbol, result, next_condition)) break;
        symbol.pop_back();
      }
    }

    Kit.CleanupDeque(container).CleanupDeque(item).CleanupDeque(symbol);

    return result;
  }

  Message EntryProvider::StartActivity(deque<string> p, Chainloader *parent) {
    using namespace Entry;
    Kit Kit;
    Message result;
    size_t size = p.size(), i = 0;
    PathMap map;
    shared_ptr<void> ptr;
    bool ignore_first_arg = false;

    auto Filling = [&](bool number = false) {
      string name = kStrEmpty;

      if (Kit.GetDataType(p.at(i)) == kTypeFunction) {
        if (this->GetName() == kStrDefineCmd || this->GetName() == kStrSetCmd) {
          if (!ignore_first_arg) {
            ptr = make_shared<string>(string(p.at(i)));
            ignore_first_arg = true;
          }
          else {
            name.append("&");
            if (p.at(i).substr(0, 2) == "__") {
              ptr = make_shared<PointWrapper>(parent->GetVariable(p.at(i)));
            }
            else {
              ptr = make_shared<PointWrapper>(FindWrapper(p.at(i), true));
            }
          }
        }
      }
      else {
        ptr = make_shared<string>(string(p.at(i)));
      }

      if (Kit.GetDataType(p.at(i)) == kTypeFunction) {
        if (this->GetName() == kStrDefineCmd && !ignore_first_arg) {
          ptr = make_shared<string>(string(p.at(i)));
          ignore_first_arg = true;
        }
        else if (this->GetName() == kStrSetCmd) {
          if (!ignore_first_arg) {
            ptr = make_shared<string>(string(p.at(i)));
            ignore_first_arg = true;
          }
          else {
            if (p.at(i).substr(0, 2) == "__") name.append("&");
            ptr = make_shared<PointWrapper>(parent->GetVariable(p.at(i)));
          }
        }
        else {
          if (p.at(i).substr(0, 2) == "__") ptr = parent->GetVariable(p.at(i)).get();
          else ptr = FindWrapper(p.at(i), true).get();
        }
      }
      else {
        ptr = make_shared<string>(string(p.at(i)));
      }
      switch (number) {
      case true:name.append(to_string(i)); break;
      case false:name.append(parameters.at(i)); break;
      }
      map.insert(PathMap::value_type(name, ptr));
    };

    if (priority == kFlagPluginEntry) {
      for (i = 0; i < parameters.size(); i++) {
        if (i >= size) map.insert(PathMap::value_type(parameters[i], nullptr));
        else Filling();
      }
      Message *msgptr = activity2(map);
      result = *msgptr;
      deleter(msgptr);
    }
    else {
      if (size == parameters.size() ) {
        for (i = 0; i < size; i++) { Filling(); }
        result = activity(map);
      }
      else if (size == kFlagAutoFill) {
        for (i = 0; i < parameters.size(); i++) {
          if (i >= size) map.insert(PathMap::value_type(parameters[i], nullptr));
          else Filling();
          result = activity(map);
        }
      }
      else if (parameters.empty() && requiredcount == kFlagAutoSize) {
        for (i = 0; i < size; i++) { Filling(true); }
        result = activity(map);
      }
      else {
        switch (requiredcount) {
        case kFlagNotDefined:result.combo(kStrFatalError, kCodeBrokenEntry, string("Illegal Entry - ").append(this->name)); break;
        default:result.combo(kStrFatalError, kCodeIllegalArgs, string("Parameter count doesn't match - ").append(this->name)); break;
        }
      }
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

  Message ChainStorage::Run(deque<string> res = deque<string>()) {
    using namespace Entry;
    Message result;
    size_t i = 0;
    size_t size = 0;
    size_t tail = 0;
    stack<size_t> nest;
    stack<size_t> tracer;
    bool next_condition = false;

    CreateMap();
    if (!res.empty()) {
      if (res.size() != parameter.size()) {
        result.combo(kStrFatalError, kCodeIllegalCall, "wrong parameter count.");
        return result;
      }
      for (i = 0; i < parameter.size(); i++) {
        CreateWrapper(parameter.at(i), res.at(i), false);
      }
    }

    if (!storage.empty()) {
      size = storage.size();
      i = 0;
      while (i < size) {
        result = storage.at(i).Start();
        if (result.GetValue() == kStrFatalError) break;
        //TODO:if-elif-else
        if (result.GetCode() == kCodeReturn) {
          result.SetCode(kCodeSuccess);
          break;
        }
        if (result.GetCode() == kCodeConditionRoot) {
          tracer.push(i);
          if (result.GetValue() == kStrFalse) {
            next_condition = true;
          }
        }
        if (result.GetCode() == kCodeConditionBranch) {
          if (!next_condition) {

          }
        }
        if (result.GetCode() == kCodeConditionLeaf) {
          if (!next_condition) {

          }
        }
        if (result.GetCode() == kCodeHeadSign && result.GetValue() == kStrTrue) {
          if (nest.empty()) nest.push(i);
          if (nest.top() == i) nest.push(i);
        }
        if (result.GetCode() == kCodeHeadSign && result.GetValue() == kStrFalse) {
          nest.pop();
          i = tail;
        }
        if (result.GetCode() == kCodeTailSign && !nest.empty()) {
          tail = i;
          i = nest.top();
        }
        i++;
      }
    }

    
    DisposeMap();
    return result;
  }

  Message Kit::ExecScriptFile(string target) {
    Message result;
    vector<Chainloader> loaders;
    size_t tail = 0;
    ScriptProvider sp(target.c_str());
    ChainStorage cs(sp);

    if (target == kStrEmpty) {
      Tracking::log(result.combo(kStrFatalError, kCodeIllegalArgs, "Empty path string."));
      return result;
    }
    InjectBasicEntries();
    cs.Run();
    Entry::ResetPlugin();
    Entry::CleanupWrapper();
    return result;
  }

  Message VersionInfo(PathMap &p) {
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    std::cout << kEngineVersion << std::endl;
    return result;
  }

  Message Quit(PathMap &p) {
    Message result(kStrEmpty, kCodeQuit, kStrEmpty);
    return result;
  }

  Message PrintOnScreen(PathMap &p) {
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    string msg = CastToString(p.at("msg"));
    std::cout << msg << std::endl;
    return result;
  }

  void Kit::Terminal() {
    using namespace Entry;
    Kit Kit;
    string buf = kStrEmpty;
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    Chainloader loader;
    auto Build = [&](string target) {return Kit.BuildStringVector(target); };
    std::cout << kEngineName << ' ' << kEngineVersion << std::endl;
    std::cout << kCopyright << ' ' << kEngineAuthor << std::endl;

    CreateMap();
    InjectBasicEntries();
    Inject("version", EntryProvider("version", VersionInfo, 0));
    Inject("quit", EntryProvider("quit", Quit, 0));
    Inject("print", EntryProvider("print", PrintOnScreen, 1, kFlagNormalEntry, Build("msg")));

    while (result.GetCode() != kCodeQuit) {
      std::cout << '>';
      std::getline(std::cin, buf);
      if (buf != kStrEmpty) {
        result = loader.Reset().Build(buf).Start();
        if (result.GetCode() < kCodeSuccess) {
          std::cout << result.GetDetail() << std::endl;
        }
      }
    }
    ResetPlugin();
    CleanupWrapper();
  }
}

