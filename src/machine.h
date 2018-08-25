#pragma once

#include <fstream>
#include "trace.h"

#if defined(_WIN32)
#include "windows.h"
#define WIN32_LEAN_AND_MEAN
#else
#include <dlfcn.h>
#endif

namespace kagami {
  class Machine {
    std::ifstream stream;
    size_t current;
    vector<Processor> storage;
    vector<string> parameters;
    stack<size_t> cycleNestStack, cycleTailStack, modeStack;
    stack<bool> conditionStack;
    size_t currentMode;
    int nestHeadCount;
    bool health;
    bool isTerminal;
    size_t endIdx;
    vector<string> defHead;
    size_t defStart;

    void DefineSign(string head);
    void ConditionRoot(bool value);
    void ConditionBranch(bool value);
    void ConditionLeaf();
    void HeadSign(bool value);
    void TailSign();
    void MakeFunction(size_t start, size_t end);
    static bool IsBlankStr(string target);
  public:
    Machine() : current(0), currentMode(kModeNormal),
    nestHeadCount(0), health(false), isTerminal(true) {}
    Machine(Machine &machine) {
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }
    Machine(Machine &&machine) : Machine(machine) {}
    Machine(vector<Processor> storage) : current(0), currentMode(kModeNormal),
      nestHeadCount(0), health(true), isTerminal(true), endIdx(0) {
      this->storage = storage;
    }
    void operator=(Machine &machine){
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }
    void operator=(Machine &&machine) {
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }

    Machine &SetParameters(vector<string> parms);
    explicit Machine(const char *target);
    Message Run(bool createManager = true);
    Message RunAsFunction(ObjectMap &p);
    void Terminal();
    void Reset();

    bool GetHealth() const { return health; }
  };

  void Activiate();
  void InitPlanners();
  Message FunctionTunnel(ObjectMap &p);
}


