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

  Message FastOrder(string name, deque<string> res) {
    Message result(kStrFatalError, kCodeIllegalCall, "Entry is not found.");
    EntryMap::iterator it = EntryMapBase.find(name);
    if (it != EntryMapBase.end()) {
      result = it->second.StartActivity(res);
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
    else if (target == kStrFor || target == kStrWhile || target == kStrEnd) {
      result = Order("cycle");
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

  int FastGetCount(string target) {
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

namespace Suzu {
  int Util::GetDataType(string target) {
    using std::regex_match;
    int result = kTypeNull;
    auto match = [&](const regex &pat) -> bool {
      return regex_match(target, pat);
    };

    if (match(kPatternFunction)) result = kTypeFunction;
    else if (match(kPatternBoolean)) result = kTypeBoolean;
    else if (match(kPatternInteger)) result = kTypeInteger;
    else if (match(kPatternDouble)) result = KTypeDouble;
    else if (match(kPatternSymbol)) result = kTypeSymbol;
    else if (match(kPatternBlank)) result = kTypeBlank;
    else if (target.front() == '"' && target.back() == '"') result = kTypeString; //temporary fix for string ploblem
    else result = kTypeNull;

    return result;
  }

  bool Util::ActivityStart(EntryProvider &provider, deque<string> container, deque<string> &item,
    size_t top, Message &msg) {
    bool rv = true;
    int code = kCodeSuccess;
    string value = kStrEmpty;
    Message temp;

    if (provider.Good()) {
      temp = provider.StartActivity(container);
      code = temp.GetCode();
      value = temp.GetValue();

      if (code < kCodeSuccess) {
        Tracking::log(temp);
      }
      msg = temp;
      if (value == kStrRedirect) {
        item.push_back(temp.GetDetail());
      }
    }
    else {
      Tracking::log(msg.combo(kStrFatalError, kCodeIllegalCall, "Activity not found"));
      rv = false;
    }

    return rv;
  }

  void Util::PrintEvents() {
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

  ScriptProvider2::ScriptProvider2(const char *target) {
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

  Message ScriptProvider2::Get() {
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
    using Tracking::log;
    Util util;
    vector<string> output;
    char binaryoptchar = NULL;
    size_t size = target.size();
    size_t i;
    string current = kStrEmpty;
    //headlock:blank characters at head of raw string,false - enable
    //allowblank:blank characters at string value and left/right of operating symbols
    //not only blanks(some special character included)
    bool headlock = false;
    bool allowblank = false;
    vector<string> list = { kStrVar, "def", "return" };
    auto ToString = [](char c) -> string {
      return string().append(1, c);
    };

    if (target == kStrEmpty) {
      log(Message(kStrWarning, kCodeIllegalArgs, "Chainloader::Build() 1"));
      return *this;
    }

    for (i = 0; i < size; i++) {
      if (!headlock) {
        if (util.GetDataType(ToString(target[i])) == kTypeBlank) {
          continue;
        }
        else if (util.GetDataType(ToString(target[i])) != kTypeBlank) {
          headlock = true;
        }
      }

      if (target[i] == '"') {
        if (allowblank && target[i - 1] != '\\' && i - 1 >= 0) {
          allowblank = !allowblank;
          current.append(1, target.at(i));
          output.push_back(current);
          current = kStrEmpty;
          continue;
        }
        else if (!allowblank) {
          allowblank = !allowblank;
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
        if (allowblank) {
          current.append(1, target[i]);
        }
        else {
          if (current != kStrEmpty) output.push_back(current);
          output.push_back(ToString(target[i]));
          current = kStrEmpty;
        }
        break;
      case '"':
        if (allowblank) {
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

        if (allowblank) {
          current.append(1, target[i]);
        }
        else {
          if (i + 1 < size && target[i + 1] == '=') {
            binaryoptchar = target[i];
            if (current != kStrEmpty) output.push_back(current);
            current = kStrEmpty;
            continue;
          }
          else if (binaryoptchar != NULL) {
            string binaryopt = { binaryoptchar, target[i] };
            if (util.GetDataType(binaryopt) == kTypeSymbol) {
              output.push_back(binaryopt);
              binaryoptchar = NULL;
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
        if (allowblank) {
          current.append(1, target[i]);
        }
        else if (util.Compare(current, list) && output.empty()) {
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
    util.CleanUpVector(output);

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

  Message Chainloader::Start() {
    using namespace Entry;
    const size_t size = raw.size();
    Util util;
    Message result, tempresult;
    size_t i, j, k;
    size_t nextinspoint = 0;
    size_t forwardtype = kTypeNull;
    string tempstr = kStrEmpty;
    int unitType = kTypeNull;
    int m = 0;
    bool entryresult = true;
    bool commaexp = false;
    bool directappend = false;
    bool forwardinsert = false;
    bool disableset = false;

    deque<string> item;
    deque<string> symbol;
    deque<string> container0;
    stack<string> container1;

    auto StartCode = [&]() -> bool {
      EntryProvider provider;
      bool reverse = true;
      util.CleanUpDeque(container0);
      provider = Entry::Find(symbol.back());
      m = provider.GetRequiredCount();
      if (provider.GetPriority() == kFlagBinEntry) {
        if (m != kFlagAutoSize) m -= 1;
        reverse = false;
      }
      if (disableset) {
        while (item.back() != ",") {
          container0.push_front(item.back());
          item.pop_back();
        }
        item.pop_back();
      }
      else {
        while (m != 0 && item.empty() != true) {
          switch (reverse) {
          case true:container0.push_front(item.back()); break;
          case false:container0.push_back(item.back()); break;
          }
          item.pop_back();
          --m;
        }
      }
      if (provider.GetPriority() == kFlagBinEntry) {
        container0.push_back(symbol.back());
      }
      return util.ActivityStart(provider, container0, item, item.size(), result);
    };

    for (i = 0; i < size; ++i) {
      unitType = util.GetDataType(raw.at(i));
      /////////////////////////////////////////////////////////////////////////////////
      if (unitType == kTypeSymbol) {
        if (raw[i] == "\"") {
          switch (directappend) {
          case true:item.back().append(raw[i]);break;
          case false:item.push_back(raw[i]);break;
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
          if (symbol.empty() || util.GetDataType(symbol.back()) == kTypeSymbol) {
            symbol.push_back("commaexp");
          }
          symbol.push_back(raw[i]);
        }
        else if (raw[i] == ")") {
          while (symbol.back() != "(" && symbol.empty() != true) {
            if (symbol.back() == ",") {
              container1.push(item.back());
              item.pop_back();
              symbol.pop_back();
            }

            if (!StartCode()) break;
            symbol.pop_back();
          }

          if (symbol.back() == "(") symbol.pop_back();
          while (!container1.empty()) {
            item.push_back(container1.top());
            container1.pop();
          }
          if (!StartCode()) break;
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
              if (k = item.size()) { k -= Entry::FastGetCount(symbol.at(j)); }
              else { k -= Entry::FastGetCount(symbol.at(j)) - 1; }
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
      /////////////////////////////////////////////////////////////////////////////////
      else if (unitType == kTypeFunction && !directappend) {
        switch (Find(raw[i]).Good()) {
        case true:symbol.push_back(raw[i]); break;
        case false:item.push_back(raw[i]); break;
        }
      }
      /////////////////////////////////////////////////////////////////////////////////
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
        util.CleanUpDeque(container0);
        if (symbol.back() == "(" || symbol.back() == ")") {
          result.combo(kStrFatalError, kCodeIllegalSymbol, "Another bracket expected.");
          break;
        }
        if (!StartCode()) break;
        symbol.pop_back();
      }
    }

    util.CleanUpDeque(container0).CleanUpDeque(item).CleanUpDeque(symbol);

    return result;
  }

  Message EntryProvider::StartActivity(deque<string> p) {
    Message result;
    size_t size = p.size();
    if (priority == kFlagPluginEntry) {
      Message *msgptr = activity2(p);
      result = *msgptr;
      delete(msgptr);
    }
    else {
      if (size == requiredcount || requiredcount == kFlagAutoSize) {
        result = activity(p);
      }
      else {
        if (requiredcount == kFlagNotDefined) {
          result.combo(kStrFatalError, kCodeBrokenEntry, string("Illegal Entry - ").append(this->name));
        }
        else {
          result.combo(kStrFatalError, kCodeIllegalArgs, string("Parameter count doesn't match - ").append(this->name));
        }
      }
    }

    return result;
  }

  bool MemoryMapper::dispose(string name) {
    bool result = true;
    if (!MapBase.empty()) {
      map<string, MemoryWrapper>::iterator it = MapBase.find(name);
      if (it != MapBase.end()) {
        it->second.free();
        MapBase.erase(it);
      }
      else result = false;
    }
    return result;
  }

  vector<string> Util::BuildStrVec(string source) {
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
    if (!temp.empty())result.push_back(temp);
    return result;
  }

  Message ChainStorage::Run(deque<string> res = deque<string>()) {
    Message result;
    size_t i = 0;
    size_t size = 0;
    size_t tail = 0;
    stack<size_t> nest;
    
    Entry::CreateMapper();
    if (!res.empty()) {
      if (res.size() != parameter.size()) {
        result.combo(kStrFatalError, kCodeIllegalCall, "wrong parameter count.");
        return result;
      }
      for (i = 0; i < parameter.size(); i++) {
        Entry::CreateWrapper(parameter.at(i), res.at(i), false);
      }
    }
    //todo:start chainloaders
    if (!storage.empty()) {
      size = storage.size();
      i = 0;
      while (i < size) {
        result = storage.at(i).Start();
        if (result.GetValue() == kStrFatalError) break;
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
      }
    }
    Entry::DisposeMapper();
    return result;
  }

  Message Util::ScriptStart(string target) {
    Message result;
    Message temp;
    size_t i;
    size_t size;
    vector<Chainloader> loaders;
    Chainloader cache;
    vector<size_t> nest;
    size_t tail = 0;

    if (target == kStrEmpty) {
      Tracking::log(result.combo(kStrFatalError, kCodeIllegalArgs, "Missing path"));
      return result;
    }
    
    TotalInjection();
    ScriptProvider2 sp(target.c_str());
    ChainStorage cs(sp);
    cs.Run();
    
    Entry::ResetPlugin();
    Entry::CleanupWrapper();
    return result;
  }

  Message VersionInfo(deque<string> &res) {
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    std::cout << kEngineVersion << std::endl;
    return result;
  }

  Message QuitTerminal(deque<string> &res){
    Message result(kStrEmpty, kCodeQuit, kStrEmpty);
    return result;
  }

  Message PrintStr(deque<string> &res) {
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    std::cout << res.at(0) << std::endl;
    return result;
  }

  void Util::Terminal() {
    using namespace Entry;
    string buf = kStrEmpty;
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    Chainloader loader;

    std::cout << kEngineName << ' ' << kEngineVersion << std::endl;
    std::cout << kCopyright << ' ' << kEngineAuthor << std::endl;

    TotalInjection();
    Inject("version", EntryProvider("version", VersionInfo, 0));
    Inject("quit", EntryProvider("quit", QuitTerminal, 0));
    Inject("print", EntryProvider("print", PrintStr, 1));
    
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


