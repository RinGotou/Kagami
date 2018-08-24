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
      if (unit != '\n' && unit != ' ' && unit != '\t' && unit != '\r') {
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

    entry::DisposeManager();

    return result;
  }

  void ScriptMachine::Terminal() {
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

