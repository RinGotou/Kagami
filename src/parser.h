#pragma once

#include <fstream>
#include "processor.h"

#if defined(_WIN32)
#include "windows.h"
#define WIN32_LEAN_AND_MEAN
#else
#include <dlfcn.h>
#endif

namespace kagami {

  /*ScriptMachine class
  Script provider caches original string data from script file.
  */
  class ScriptMachine {
    std::ifstream stream;
    size_t current;
    vector<Processor> storage;
    vector<string> parameters;
    bool end;
    stack<size_t> cycleNestStack, cycleTailStack, modeStack;
    stack<bool> conditionStack;
    size_t currentMode;
    int nestHeadCount;
    bool health;
    bool isTerminal;
    size_t endIdx;

    void ConditionRoot(bool value);
    void ConditionBranch(bool value);
    void ConditionLeaf();
    void HeadSign(bool value, bool selfObjectManagement);
    void TailSign();
    static bool IsBlankStr(string target);
  public:
    ScriptMachine() : current(0), end(false), currentMode(kModeNormal),
    nestHeadCount(0), health(false), isTerminal(true) {}
    ScriptMachine(vector<Processor> storage) : current(0), end(false), currentMode(kModeNormal),
      nestHeadCount(0), health(true), isTerminal(true), endIdx(0) {
      this->storage = storage;
    }

    explicit ScriptMachine(const char *target);
    Message Run();
    void Terminal();

    bool GetHealth() const { return health; }
    bool Eof() const { return end; }
    void ResetCounter() {
      current = 0;
      end = false;
    }
  };

  void Activiate();
  void InitPlanners();



  namespace trace {
    using log_t = pair<string, Message>;
    void Log(kagami::Message msg);
    vector<log_t> &GetLogger();
  }
}


