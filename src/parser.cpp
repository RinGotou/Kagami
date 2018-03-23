#include "parser.h"

namespace Tracking {
  using namespace Suzu;
  vector<Message> base;

  void log(Message msg) {
    base.push_back(msg);
  }

  bool IsEmpty() {
    return base.empty();
  }
}

namespace Entry {
  using namespace Suzu;
  deque<EntryProvider> base;

  void Inject(EntryProvider provider) {
    base.push_back(provider);
  }

  Message FastOrder(string name, vector<string> &res) {
    Message result(kStrFatalError, kCodeIllegalCall, "Entry Not Found.");
    for (auto &unit : base) {
      if (unit.GetName() == name) result = unit.StartActivity(res);
    }
    return result;
  }

  EntryProvider Order(string name) {
    EntryProvider result;
    for (auto &unit : base) {
      if (unit.GetName() == name) result = unit;
    }
    return result;
  }

  EntryProvider Query(string target) {
    EntryProvider result;
    if (target == "+" || target == "-" || target == "*" || target == "/") {
      result = Order("binexp");
    }
    else {
      for (auto &unit : base) {
        if (unit.GetName() == target && unit.GetPriority() != 0) result = unit;
      }
    }
    return result;
  }

  int FastGetCount(string target) {
    for (auto &unit : base) {
      if (unit.GetName() == target) {
        return unit.GetRequiredCount();
      }
    }
    return kFlagNotDefined;
  }
}

namespace Suzu {
  Message Util::GetDataType(string target) {
    using std::regex_match;
    Message result(kStrRedirect, kCodeIllegalArgs,"");
    auto match = [&](const regex &pat) -> bool {
      return regex_match(target, pat);
    };

    if (match(kPatternFunction)) result.SetCode(kTypeFunction);
    else if (match(kPatternString)) result.SetCode(kTypeString);
    else if (match(kPatternBoolean)) result.SetCode(kTypeBoolean);
    else if (match(kPatternInteger)) result.SetCode(kTypeInteger);
    else if (match(kPatternDouble)) result.SetCode(KTypeDouble);
    else if (match(kPatternSymbol)) result.SetCode(kTypeSymbol);
    else result.SetDetail("No match type.");
    return result;
  }

  bool Util::ActivityStart(EntryProvider &provider, vector<string> container, deque<string> &item,
    size_t top, Message &msg) {
    bool rv = true;
    int code = kCodeSuccess;
    Message temp;

    if (provider.Good()) {
      temp = provider.StartActivity(container);
      code = temp.GetCode();

      if (code < kCodeSuccess) {
        Tracking::log(temp.SetDetail("ActivityStart 1"));
        msg = temp;
        rv = false;
      }
      else if (code == kCodeRedirect) {
        if (top == item.size()) {
          item.push_back(temp.GetValue());
        }
        else {
          item[top] = temp.GetValue();
        }
      }
      else {
        if (top == item.size()) {
          item.push_back(kStrEmpty);
        }
        else {
          item[top] = kStrEmpty;
        }
      }
    }
    else {
      Tracking::log(msg.combo(kStrFatalError, kCodeIllegalCall, "ActivityStart 2"));
      rv = false;
    }

    return rv;
  }

  void Util::PrintEvents() {
    using namespace Tracking;
    ofstream ofs("event.log", std::ios::trunc);
    size_t i = 0;
    if (ofs.good()) {
      if (base.empty()) {
        ofs << "No Events\n";
      }
      else {
        for (auto unit : base) {
          ++i;
          ofs << "Count:" << i << "\n" << "Code:" << unit.GetCode() << "\n";
          if (unit.GetValue() == kStrFatalError) 
            ofs << "Priority:Fatal\n";
          else if (unit.GetValue() == kStrWarning)
            ofs << "Priority:Warning\n";
          if (unit.GetDetail() != kStrEmpty) 
            ofs << "Detail:" << unit.GetDetail() << "\n";
        }
      }
    }
    ofs.close();
  }

  void Util::Cleanup() {
    //nothing to dispose here
  }

  Message ScriptProvider::Get() {
    using Tracking::log;
    string currentstr;
    Message result(kStrEmpty, kCodeSuccess, "");

    auto IsBlankStr = [](string target) -> bool {
      if (target == kStrEmpty || target.size() == 0) return true;
      for (const auto unit : target) {
        if (unit != ' ' || unit != '\n' || unit != '\t' || unit != '\r') {
          return false;
        }
      }
      return true;
    };

    if (pool.empty() && IsStreamReady()) {
      while (!(stream.eof())) {
        std::getline(stream, currentstr);
        if (IsBlankStr(currentstr) != true) {
          pool.push_back(currentstr);
        }
      }
      stream.close();
    }
    else {
      log(result.combo(kStrFatalError, kCodeBadStream, "ScriptProvder::Get() 1"));
      return result;
    }

    size_t size = pool.size();
    if (current < size) {
      result.SetValue(pool[current]);
      current++;
    }
    else if (current == size) {
      result.SetValue(kStrEOF);
    }
    else {
      log(result.combo(kStrFatalError, kCodeOverflow, "ScriptProvder::Get() 2"));
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
    vector<string> list = { kStrVar,kstrDefine,kStrReturn };
    auto ToString = [](char c) -> string {
      return string().append(1, c);
    };

    if (target == kStrEmpty) {
      log(Message(kStrWarning, kCodeIllegalArgs,"Chainloader::Build() 1"));
      return *this;
    }

    for (i = 0; i < size; i++) {
      if (headlock == false && std::regex_match(ToString(target[i]),
        kPatternBlank)) {
        continue;
      }
      else if (headlock == false && std::regex_match(ToString(target[i]),
        kPatternBlank) == false) {
        headlock = true;
      }


      if (target[i] == '"') {
        if (allowblank && target[i - 1] != '\\' && i - 1 >= 0) {
          allowblank = !allowblank;
        }
        else if (!allowblank) {
          allowblank = !allowblank;
        }
      }

      //In this verison comma is preserved for next execution processing.
      switch (target[i]) {
      case '(':
      case ',':
      case ')':
      case '{':
      case '}':
      case ':':
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
        if (allowblank && target[i - 1] == '\\' && i - 1 >= 0) {
          current.append(1, target[i]);
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
            if (util.GetDataType(binaryopt).GetCode() == kTypeSymbol) {
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
        else if (util.Compare(current,list) && output.empty()){
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

    raw = output;
    util.CleanUpVector(output);

    return *this;
  }

  //private
  int Chainloader::GetPriority(string target) const {
    if (target == "+" || target == "-") return 1;
    if (target == "*" || target == "/" || target == "\\") return 2;
    return 3;
  }

  //todo:comma expression expanding
  Message Chainloader::Start() {
    const size_t size = raw.size();

    Util util;
    EntryProvider provider;
    Message result, tempresult;
    size_t i, j, k, m;
    size_t nextinspoint = 0;
    string tempstr;
    bool entryresult = true;
    bool commaexp = false;
    bool directappend = false;
    size_t forwardtype = kTypeNull;
    deque<string> item;
    deque<string> symbol;
    vector<string> container0;
    stack<string> container1;

    auto StartCode = [&provider, &j, &item, &container0, &util, &result]()->bool {
      j = provider.GetRequiredCount();
      while (j != 0 && item.empty() != true) {
        container0.push_back(item.back());
        item.pop_back();
        --j;

      }
      return util.ActivityStart(provider, container0, item, item.size(), result);
    };

    for (i = 0; i < size; ++i) {
      if (regex_match(raw[i], kPatternSymbol)) {
        if (raw[i] == ",") {
          symbol.push_back(raw[i]);
        }
        if (raw[i] == "(") {
          if (forwardtype == kTypeNull || forwardtype == kTypeSymbol) {
            commaexp = true;
          }
          symbol.push_back(raw[i]);
        }
        else if (raw[i] == ")") {
          //Need to modify!
          while (symbol.back() != "(" && symbol.empty() != true) {
            if (symbol.back() == ",") {
              //backup current item
              container1.push(item.back());
              item.pop_back();
              symbol.pop_back();
            }
            util.CleanUpVector(container0);
            provider = Entry::Query(symbol.back());
            entryresult = StartCode();
            if (entryresult == false) break;

            if (result.GetValue() == kStrRedirect) {
              //TODO:push function result back to item deque
              item.push_back(result.GetDetail);
            }
            else {
              switch (result.GetCode()) {
              case kCodeSuccess:
                item.push_back(kStrTrue);
                break;
              default:
                item.push_back(kStrFalse);
              }
            }
            symbol.pop_back();
          }
          
          if (symbol.back() == "(") symbol.pop_back();
          if (container1.empty() != true) {
            item.push_back(container1.top());
            container1.pop();
          }
          provider = Entry::Query(symbol.back());
          entryresult = StartCode();
          if (entryresult == false) break;
        }
        else {
          if (GetPriority(raw[i]) < GetPriority(symbol.back())) {
            j = symbol.size() - 1;
            k = item.size() - 1;
            while (symbol.at(j) != "(" || GetPriority(raw[i]) < GetPriority(symbol.at(j))) {
              if (k = item.size() - 1) k -= Entry::FastGetCount(symbol.at(j));
              else k -= Entry::FastGetCount(symbol.at(j)) - 1;
                --j;
            }
            symbol.insert(symbol.begin() + j, raw[i]);
            nextinspoint = k;

            j = 0;
            k = 0;
          }
          else {
            symbol.push_back(raw[i]);
          }
        }
      }
      else if (regex_match(raw[i], kPatternFunction)) {
        provider = Entry::Query(raw[i]);
        if (provider.Good()) {
          symbol.push_back(raw[i]);
        }
        else {
          item.push_back(raw[i]);
        }
      }
      else {
        if (nextinspoint != 0) {
          item.insert(item.begin() + nextinspoint, raw[i]);
          nextinspoint = 0;
        }
        else {
          item.push_back(raw[i]);
        }
      }
    }

    while (symbol.empty() != true) {
      util.CleanUpVector(container0);
      if (item.empty()) {
        result.combo(kStrFatalError, kCodeIllegalSymbol, "Parameters expected.");
        break;
      }
      if (symbol.back() == "(" || symbol.back() == ")") {
        result.combo(kStrFatalError, kCodeIllegalSymbol, "Another bracket expected.");
        break;
      }
      provider = Entry::Query(symbol.back());
      entryresult = StartCode();
      if (entryresult == false) break;
      symbol.pop_back();
    }

    util.CleanUpVector(container0).CleanUpDeque(item).CleanUpDeque(symbol);

    return result;
  }

  Message EntryProvider::StartActivity(vector<string> p) {
    Message result;
    size_t size = p.size();

    if (size == requiredcount || requiredcount == kFlagAutoSize) {
      result = activity(p);
    }
    else {
      if (requiredcount == kFlagNotDefined) {
        Tracking::log(result.combo(kStrFatalError, kCodeBrokenEntry, 
          string("Illegal Entry - ").append(this->name)));
      }
      else {
        Tracking::log(result.combo(kStrFatalError, kCodeIllegalArgs,
          string("Parameter count doesn't match - ").append(this->name)));
      }
    }
		
    return result;
  }

  Message Util::ScriptStart(string target) {
    Message result;
    Message temp;
    size_t i;
    size_t size;
    vector<Chainloader> loaders;
    Chainloader cache;

    if (target == kStrEmpty) {
      Tracking::log(result.combo(kStrFatalError, kCodeIllegalArgs, "Util::ScriptStart() 1"));
    }
    else {
      TotalInjection();
      ScriptProvider sp(target);
      while (!sp.eof()) {
        temp = sp.Get();
        if (temp.GetCode() == kCodeSuccess) {
          cache.Reset().Build(temp.GetValue());
          loaders.push_back(cache);
        }
        else {
          Tracking::log(result.combo(kStrFatalError, kCodeIllegalArgs, "Util::ScriptStart() 2"));
          break;
        }
      }

      if (!loaders.empty()) {
        size = loaders.size();
        for (i = 0; i < size; i++) {
          temp = loaders[i].Start();
          if (temp.GetCode() != kCodeSuccess) {
            if (temp.GetValue() == kStrFatalError) break;
          }
        }
      }
    }

    return result;
  }

  StrPair *MemoryProvider::find(string name) {
    StrPair *result = nullptr;
    for (auto &unit : dict) {
      if (unit.first == name) {
        result = &unit;
        break;
      }
    }
    return result;
  }

  bool MemoryProvider::dispose(string name) {
    bool result = true;
    MemPtr ptr;
    if (dict.empty() == false) {
      ptr = dict.begin();
      while (ptr != dict.end() && ptr->first != name) ++ptr;
      if (ptr == dict.end() && ptr->first != name) result = false;
      else {
        dict.erase(ptr);
      }
    }
    return result;
  }

  string MemoryProvider::query(string name) {
    string result;
    StrPair *ptr = find(name);
    if (ptr != nullptr) result = ptr->second;
    else result = kStrNull;
    return result;
  }

  string MemoryProvider::set(string name, string value) {
    string result;
    StrPair *ptr = find(name);
    if (ptr != nullptr && ptr->IsReadOnly() != true) {
      result = ptr->second;
      ptr->second = value;
    }
    else {
      result = kStrNull;
    }
    return result;
  }
}


