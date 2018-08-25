//#include <iostream>
#include <ctime>
#ifndef _NO_CUI_
#include <iostream>
#endif
#include "machine.h"

namespace kagami {
  map<string, Machine> &GetFunctionBase() {
    static map<string, Machine> base;
    return base;
  }

  inline void AddFunction(string id, vector<Processor> proc, vector<string> parms) {
    auto &base = GetFunctionBase();
    base[id] = Machine(proc).SetParameters(parms);
    entry::AddEntry(Entry(FunctionTunnel, id, parms));
  }

  inline Machine *GetFunction(string id) {
    Machine *machine = nullptr;
    auto &base = GetFunctionBase();
    auto it = base.find(id);
    if (it != base.end()) machine = &(it->second);
    return machine;
  }

  Message FunctionTunnel(ObjectMap &p) {
    Message msg;
    Object &funcId = p[kStrUserFunc];
    string id = *static_pointer_cast<string>(funcId.Get());
    Machine *machine = GetFunction(id);
    if (machine != nullptr) {
      msg = machine->RunAsFunction(p);
    }
    return msg;
  }

  inline bool GetBooleanValue(string src) {
    if (src == kStrTrue) return true;
    if (src == kStrFalse) return false;
    if (src == "0" || src.empty()) return false;
    return true;
  }

  void Machine::MakeFunction(size_t start,size_t end) {
    if (start > end) return;
    string id = defHead[0];
    vector<string> parms;
    vector<Processor> proc;
    for (size_t i = 1; i < defHead.size(); ++i) {
      parms.push_back(defHead[i]);
    }
    for (size_t j = start; j <= end; ++j) {
      proc.push_back(storage[j]);
    }
    AddFunction(id, proc, parms);
  }

  Machine &Machine::SetParameters(vector<string> parms) {
    this->parameters = parms;
    return *this;
  }

  void Machine::Reset() {
    current = 0;
    currentMode = kModeNormal;
    nestHeadCount = 0;
    isTerminal = false;
    Kit().CleanupVector(storage);
    while (!cycleNestStack.empty()) cycleNestStack.pop();
    while (!cycleTailStack.empty()) cycleTailStack.pop();
    while (!modeStack.empty()) modeStack.pop();
    while (!conditionStack.empty()) conditionStack.pop();
  }

  Machine::Machine(const char *target) {
    string temp;
    isTerminal = false;
    health = true;
    current = 0;
    endIdx = 0;
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

  void Machine::DefineSign(string head) {
    defHead = Kit::BuildStringVector(head);
    if (currentMode != kModeDef && currentMode == kModeNormal) {
      currentMode = kModeDef;
      modeStack.push(kModeNormal);
      defStart = current + 1;
    }
    else if (currentMode != kModeNormal) {
      //?
    }
  }

  void Machine::ConditionRoot(bool value) {
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

  void Machine::ConditionBranch(bool value) {
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

  void Machine::ConditionLeaf() {
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

  void Machine::HeadSign(bool value) {
    if (cycleNestStack.empty()) {
      modeStack.push(currentMode);
      entry::CreateManager();
    }
    else {
      if (cycleNestStack.top() != current - 1) {
        modeStack.push(currentMode);
         entry::CreateManager();
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

  void Machine::TailSign() {
    if (currentMode == kModeDef) {
      MakeFunction(defStart, current - 1);
      currentMode = modeStack.top();
      modeStack.pop();
      Kit().CleanupVector(defHead);
    }
    else if (currentMode == kModeCondition || currentMode == kModeNextCondition) {
      conditionStack.pop();
      currentMode = modeStack.top();
      modeStack.pop();
      entry::DisposeManager();
    }
    else if (currentMode == kModeCycle || currentMode == kModeCycleJump) {
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

  bool Machine::IsBlankStr(string target) {
    if (target == kStrEmpty || target.size() == 0) return true;
    for (const auto unit : target) {
      if (unit != '\n' && unit != ' ' && unit != '\t' && unit != '\r') {
        return false;
      }
    }
    return true;
  }

  Message Machine::Run(bool createManager) {
    Message result;

    currentMode = kModeNormal;
    nestHeadCount = 0;
    current = 0;
    health = true;

    if (storage.empty()) return result;

    if (createManager) entry::CreateManager();

    //Main state machine
    while (current < storage.size()) {
      if (!health) break;

      result = storage[current].Activiate(currentMode);
      const auto value = result.GetValue();
      const auto code  = result.GetCode();

      if (value == kStrFatalError) {
        trace::Log(result.SetIndex(storage[current].GetIndex()));
        break;
      }

      if (value == kStrWarning) {
        trace::Log(result.SetIndex(storage[current].GetIndex()));
      }
      //TODO:return

      switch (code) {
      case kCodeDefineSign:
        DefineSign(result.GetDetail());
        break;
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
        HeadSign(GetBooleanValue(value));
        break;
      case kCodeHeadPlaceholder:
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

    if (createManager) entry::DisposeManager();

    return result;
  }

  Message Machine::RunAsFunction(ObjectMap &p) {
    Message msg;
    auto &base = entry::CreateManager();
    for (auto &unit : p) {
      base.Add(unit.first, unit.second);
    }
    msg = Run(false);
    if (msg.GetCode() >= kCodeSuccess) {
      msg = Message();
      Object *ret = base.Find(kStrRetValue);
      if (ret != nullptr) {
        Object obj;
        obj.Copy(*ret);
        msg.SetObject(obj, "__result");
      }
    }
    return msg;
  }

  void Machine::Terminal() {
    Message msg;
    string buf;
    string head = kStrNormalArrow;
    Processor processor;
    bool subProcess = false;
    bool skip = false;

    entry::CreateManager();

    while (msg.GetCode() != kCodeQuit) {
      msg = Message();
      std::cout << head;
      std::getline(std::cin, buf);
      if (buf.empty()) continue;
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
        skip = true;
        storage.emplace_back(processor);
        head = kStrNormalArrow;
        msg = this->Run();
        Kit().CleanupVector(storage);
        //storage.clear();
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
        if(!skip) msg = processor.Activiate();
      }

      if (msg.GetCode() < kCodeSuccess) {
        std::cout << msg.GetDetail() << std::endl;
        trace::Log(msg);
      }
      if (skip == true) skip = false;
    }

    entry::DisposeManager();
  }
}

