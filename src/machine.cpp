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
    Machine *mach = GetFunction(id);
    if (mach != nullptr) {
      msg = mach->RunAsFunction(p);
    }
    return msg;
  }

  inline bool GetBooleanValue(string src) {
    if (src == kStrTrue) return true;
    if (src == kStrFalse) return false;
    if (src == "0" || src.empty()) return false;
    return true;
  }

  void Machine::MakeFunction(size_t start,size_t end, MachCtlBlk *blk) {
    if (start > end) return;
    auto &defHead = blk->defHead;
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

  void Machine::Reset(MachCtlBlk *blk) {
    Kit().CleanupVector(storage);
    while (!blk->cycleNestStack.empty()) blk->cycleNestStack.pop();
    while (!blk->cycleTailStack.empty()) blk->cycleTailStack.pop();
    while (!blk->modeStack.empty()) blk->modeStack.pop();
    while (!blk->conditionStack.empty()) blk->conditionStack.pop();
    delete blk;
  }

  Machine::Machine(const char *target) {
    std::ifstream stream;
    string temp;
    health = true;
    size_t subscript = 0;

    stream.open(target, std::ios::in);
    if (stream.good()) {
      while (!stream.eof()) {
        std::getline(stream, temp);
        if (!IsBlankStr(temp) && temp.front() != '#') {
          storage.emplace_back(Processor().Build(temp).SetIndex(subscript));
        }
        subscript++;
      }
    }
    stream.close();
  }

  void Machine::DefineSign(string head, MachCtlBlk *blk) {
    blk->defHead = Kit::BuildStringVector(head);
    if (blk->currentMode != kModeDef && blk->currentMode == kModeNormal) {
      blk->currentMode = kModeDef;
      blk->modeStack.push(kModeNormal);
      blk->defStart = blk->current + 1;
    }
    else if (blk->currentMode != kModeNormal) {
      //?
    }
  }

  void Machine::ConditionRoot(bool value, MachCtlBlk *blk) {
    blk->modeStack.push(blk->currentMode);
    entry::CreateManager();
    if (value == true) {
      blk->currentMode = kModeCondition;
      blk->conditionStack.push(true);
    }
    else {
      blk->currentMode = kModeNextCondition;
      blk->conditionStack.push(false);
    }
  }

  void Machine::ConditionBranch(bool value, MachCtlBlk *blk) {
    if (!blk->conditionStack.empty()) {
      if (blk->conditionStack.top() == false && blk->currentMode == kModeNextCondition
        && value == true) {
        entry::CreateManager();
        blk->currentMode = kModeCondition;
        blk->conditionStack.top() = true;
      }
      else if (blk->conditionStack.top() == true && blk->currentMode == kModeCondition) {
        blk->currentMode = kModeNextCondition;
      }
    }
    else {
      //msg.combo
      health = false;
    }
  }

  void Machine::ConditionLeaf(MachCtlBlk *blk) {
    if (!blk->conditionStack.empty()) {
      if (blk->conditionStack.top() == true) {
        blk->currentMode = kModeNextCondition;
      }
      else {
        entry::CreateManager();
        blk->conditionStack.top() = true;
        blk->currentMode = kModeCondition;
      }
    }
    else {
      //msg.combo
      health = false;
    }
  }

  void Machine::HeadSign(bool value, MachCtlBlk *blk) {
    if (blk->cycleNestStack.empty()) {
      blk->modeStack.push(blk->currentMode);
      entry::CreateManager();
    }
    else {
      if (blk->cycleNestStack.top() != blk->current - 1) {
        blk->modeStack.push(blk->currentMode);
        entry::CreateManager();
      }
    }
    if (value == true) {
      blk->currentMode = kModeCycle;
      if (blk->cycleNestStack.empty()) {
        blk->cycleNestStack.push(blk->current - 1);
      }
      else if (blk->cycleNestStack.top() != blk->current - 1) {
        blk->cycleNestStack.push(blk->current - 1);
      }
    }
    else if (value == false) {
      blk->currentMode = kModeCycleJump;
      if (!blk->cycleTailStack.empty()) {
        blk->current = blk->cycleTailStack.top();
      }
    }
    else {
      health = false;
    }
  }

  void Machine::TailSign(MachCtlBlk *blk) {
    if (blk->currentMode == kModeDef) {
      MakeFunction(blk->defStart, blk->current - 1, blk);
      blk->currentMode = blk->modeStack.top();
      blk->modeStack.pop();
      Kit().CleanupVector(blk->defHead);
    }
    else if (blk->currentMode == kModeCondition || blk->currentMode == kModeNextCondition) {
      blk->conditionStack.pop();
      blk->currentMode = blk->modeStack.top();
      blk->modeStack.pop();
      entry::DisposeManager();
    }
    else if (blk->currentMode == kModeCycle || blk->currentMode == kModeCycleJump) {
      switch (blk->currentMode) {
      case kModeCycle:
        if (blk->cycleTailStack.empty() || blk->cycleTailStack.top() != blk->current - 1) {
          blk->cycleTailStack.push(blk->current - 1);
        }
        blk->current = blk->cycleNestStack.top();
        entry::GetCurrentManager().clear();
        break;
      case kModeCycleJump:
        blk->currentMode = blk->modeStack.top();
        blk->modeStack.pop();
        blk->cycleNestStack.pop();
        if (!blk->cycleTailStack.empty()) blk->cycleTailStack.pop();
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
    MachCtlBlk *blk = new MachCtlBlk();

    blk->currentMode = kModeNormal;
    blk->nestHeadCount = 0;
    blk->current = 0;
    blk->currentMode = kModeNormal;
    blk->defStart = 0;
    health = true;

    if (storage.empty()) return result;

    if (createManager) entry::CreateManager();

    //Main state machine
    while (blk->current < storage.size()) {
      if (!health) break;

      result = storage[blk->current].Activiate(blk->currentMode);
      const auto value = result.GetValue();
      const auto code  = result.GetCode();

      if (value == kStrFatalError) {
        trace::Log(result.SetIndex(storage[blk->current].GetIndex()));
        break;
      }

      if (value == kStrWarning) {
        trace::Log(result.SetIndex(storage[blk->current].GetIndex()));
      }

      if (value == kStrStopSign) {
        break;
      }

      switch (code) {
      case kCodeDefineSign:
        DefineSign(result.GetDetail(), blk);
        break;
      case kCodeConditionRoot:
        ConditionRoot(GetBooleanValue(value), blk); 
        break;
      case kCodeConditionBranch:
        if (blk->nestHeadCount > 0) break;
        ConditionBranch(GetBooleanValue(value), blk);
        break;
      case kCodeConditionLeaf:
        if (blk->nestHeadCount > 0) break;
        ConditionLeaf(blk);
        break;
      case kCodeHeadSign:
        HeadSign(GetBooleanValue(value), blk);
        break;
      case kCodeHeadPlaceholder:
        blk->nestHeadCount++;
        break;
      case kCodeTailSign:
        if (blk->nestHeadCount > 0) {
          blk->nestHeadCount--;
          break;
        }
        TailSign(blk);
        break;
      default:break;
      }
      ++blk->current;
    }

    if (createManager) entry::DisposeManager();
    delete blk;
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
    Object *funcSign = entry::GetCurrentManager().Find(kStrUserFunc);
    string funcId = *static_pointer_cast<string>(p[kStrUserFunc].Get());
    while (funcSign == nullptr) {
      entry::DisposeManager();
      funcSign = entry::GetCurrentManager().Find(kStrUserFunc);
      if (funcSign != nullptr) {
        string value = *static_pointer_cast<string>(p[kStrUserFunc].Get());
        if (value == funcId) break;
      }
    }
    entry::DisposeManager();
    return msg;
  }
}

