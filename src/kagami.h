#pragma once
#include "machine.h"

namespace kagami {
  class ScriptCore {
  private:
    bool ignoreFatalError;
  public:
    void PrintEvents(const char *path, const char *scriptPath);
    void ExecScriptFile(string target);
    void MyInfo();
  };
}