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
  template <class T>
  T &GetObjectStuff(Object &obj) {
    return *static_pointer_cast<T>(obj.Get());
  }

  using MachCtlBlk = struct {
    size_t current;
    stack<size_t> cycleNestStack, cycleTailStack, modeStack;
    stack<bool> conditionStack;
    size_t currentMode;
    int nestHeadCount;
    vector<string> defHead;
    size_t defStart;
  };

  class Machine {
    vector<Processor> storage;
    vector<string> parameters;
    bool health;

    void DefineSign(string head, MachCtlBlk *blk);
    void ConditionRoot(bool value, MachCtlBlk *blk);
    void ConditionBranch(bool value, MachCtlBlk *blk);
    void ConditionLeaf(MachCtlBlk *blk);
    void HeadSign(bool value, MachCtlBlk *blk);
    void TailSign(MachCtlBlk *blk);

    void MakeFunction(size_t start, size_t end, MachCtlBlk *blk);
    static bool IsBlankStr(string target);
  public:
    Machine() : health(false) {}
    Machine(Machine &machine) {
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }
    Machine(Machine &&machine) : Machine(machine) {}
    Machine(vector<Processor> storage) : health(true) {
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
    void Reset(MachCtlBlk *blk);

    bool GetHealth() const { return health; }
  };

  void Activiate();
  void InitPlanners();
  Message FunctionTunnel(ObjectMap &p);
  Message Calling(Activity activity, string args, vector<Object> objects);
}


