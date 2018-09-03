#pragma once
#include "machine.h"

namespace kagami {
  /*Core Class
  this class contains all main functions of script processor.
  */
  class ScriptCore {
  private:
    bool ignoreFatalError;
  public:
    void PrintEvents();
    Message ExecScriptFile(string target);
  };
}